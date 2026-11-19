#include "bluetooth_client.h"
#include "bluetooth.h"
#include <QMetaEnum>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#endif

using namespace Qt::StringLiterals;

static const QBluetoothUuid serviceUuid(QStringLiteral("e8e10f95-1a70-4b27-9ccf-02010264e9c8"));

BluetoothClient::BluetoothClient(QObject *parent)
    : QObject(parent)
{
    m_connectionTimer.setSingleShot(true);
    m_connectionTimer.setInterval(20000);
    connect(&m_connectionTimer, &QTimer::timeout, this, [this]() {
        const QString address = m_targetAddress;
        releaseSocket();
        emit socketErrorOccurred(tr("Connection to %1 timed out.").arg(address));
    });
}

void BluetoothClient::ensureDiscoveryAgent() {
    if (discoveryAgent) return;
    // On Android the agent must be created after Bluetooth permission is granted.
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothClient::onDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothClient::onDiscoveryFinished);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,
            this, &BluetoothClient::onDiscoveryFinished);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this,
            [this](QBluetoothDeviceDiscoveryAgent::Error) {
        emit socketErrorOccurred(tr("Device search failed: %1").arg(discoveryAgent->errorString()));
        emit discoveringChanged();
    });
}

BluetoothClient::~BluetoothClient() {
    stopClient();
}

void BluetoothClient::addDevice(const QString &name, const QString &address, bool paired) {
    for (int i = 0; i < foundDevices.size(); ++i) {
        QVariantMap item = foundDevices[i].toMap();
        if (item["address"].toString() == address) {
            if (!name.isEmpty()) item["name"] = name;
            item["paired"] = paired || item["paired"].toBool();
            foundDevices[i] = item;
            emit devicesDiscovered(foundDevices);
            return;
        }
    }

    QVariantMap deviceMap;
    deviceMap["name"] = name.isEmpty() ? u"Unknown device"_s : name;
    deviceMap["address"] = address;
    deviceMap["paired"] = paired;
    foundDevices.append(deviceMap);
    emit devicesDiscovered(foundDevices);
}

void BluetoothClient::setSavedDevice(const QString &address, const QString &name) {
    m_savedAddress = address;
    m_savedName = name;
    if (!address.isEmpty()) addDevice(name, address, false);
}

// On Android the device search finds only devices that are visible right now.
// Paired devices usually are not visible, so add them from the system list.
void BluetoothClient::addPairedDevices() {
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    QJniObject adapter = QJniObject::callStaticObjectMethod(
        "android/bluetooth/BluetoothAdapter", "getDefaultAdapter",
        "()Landroid/bluetooth/BluetoothAdapter;");
    if (!adapter.isValid()) {
        env.checkAndClearExceptions();
        return;
    }

    QJniObject bonded = adapter.callObjectMethod("getBondedDevices", "()Ljava/util/Set;");
    if (env.checkAndClearExceptions() || !bonded.isValid()) return;

    QJniObject array = bonded.callObjectMethod("toArray", "()[Ljava/lang/Object;");
    if (env.checkAndClearExceptions() || !array.isValid()) return;

    auto jarray = array.object<jobjectArray>();
    const jsize count = env->GetArrayLength(jarray);
    for (jsize i = 0; i < count; ++i) {
        QJniObject device = QJniObject::fromLocalRef(env->GetObjectArrayElement(jarray, i));
        const QString name = device.callObjectMethod<jstring>("getName").toString();
        const QString address = device.callObjectMethod<jstring>("getAddress").toString();
        env.checkAndClearExceptions();
        if (!address.isEmpty())
            addDevice(name, address, true);
    }
#endif
}

void BluetoothClient::startDiscovery() {
    ensureDiscoveryAgent();
    if (discoveryAgent->isActive())
        discoveryAgent->stop();

    foundDevices.clear();
    emit devicesDiscovered(foundDevices);

    if (!m_savedAddress.isEmpty()) addDevice(m_savedName, m_savedAddress, false);
    addPairedDevices();

    // Only classic Bluetooth devices can open an RFCOMM connection
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
    emit discoveringChanged();
    qDebug() << "Bluetooth: searching for devices...";
}

void BluetoothClient::onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
    if (!(device.coreConfigurations() & QBluetoothDeviceInfo::BaseRateCoreConfiguration))
        return;
    addDevice(device.name(), device.address().toString(), false);
}

void BluetoothClient::onDiscoveryFinished() {
    qDebug() << "Bluetooth: search finished," << foundDevices.count() << "devices";
    emit discoveringChanged();
}

bool BluetoothClient::isDiscovering() const {
    return discoveryAgent && discoveryAgent->isActive();
}

bool BluetoothClient::isConnected() {
    return socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState;
}

bool BluetoothClient::isConnecting() const {
    return socket && socket->state() != QBluetoothSocket::SocketState::ConnectedState
                  && socket->state() != QBluetoothSocket::SocketState::UnconnectedState;
}

QString BluetoothClient::connectedAddress() const {
    if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState)
        return m_targetAddress;
    return QString();
}

QString BluetoothClient::peerName() const {
    if (!socket) return QString();
    return socket->peerName().isEmpty() ? m_targetAddress : socket->peerName();
}

void BluetoothClient::connectToDevice(const QString &addressStr) {
    emit connectionRequested();
    openConnection(addressStr);
}

void BluetoothClient::reconnectToDevice(const QString &addressStr) {
    openConnection(addressStr);
}

void BluetoothClient::openConnection(const QString &addressStr) {
    const QBluetoothAddress address(addressStr);
    if (address.isNull()) {
        emit socketErrorOccurred(tr("Invalid Bluetooth device address."));
        return;
    }
    if (m_targetAddress == address.toString() && (isConnected() || isConnecting())) return;

    // Drop the old connection first, so a failed attempt can be repeated
    releaseSocket();

    // Searching slows down connecting a lot, stop it
    if (discoveryAgent && discoveryAgent->isActive())
        discoveryAgent->stop();

    m_targetAddress = address.toString();
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);

    m_transfer = new BluetoothTransfer(socket, socket);
    connect(m_transfer, &BluetoothTransfer::itemReceived, this, [this](const QVariantMap &item) {
        emit dataReceived(item);
        if (item.value("type").toString() == "text")
            emit messageReceived(peerName(), item.value("data").toString());
    });
    connect(m_transfer, &BluetoothTransfer::transferError, this, &BluetoothClient::transferError);
    connect(m_transfer, &BluetoothTransfer::finished, this, &BluetoothClient::transferFinished);
    connect(socket, &QBluetoothSocket::connected, this, &BluetoothClient::connectedSlot);
    connect(socket, &QBluetoothSocket::disconnected, this, &BluetoothClient::onSocketDisconnected);
    connect(socket, &QBluetoothSocket::errorOccurred, this, &BluetoothClient::onSocketErrorOccurred);
    connect(socket, &QBluetoothSocket::stateChanged, this, &BluetoothClient::stateChanged);

    m_connectionTimer.start();
    socket->connectToService(address, serviceUuid);
    qDebug() << "Bluetooth: connecting to" << addressStr;
    emit stateChanged();
}

void BluetoothClient::disconnectFromDevice() {
    emit disconnectRequested();
    releaseSocket();
    emit disconnected();
}

void BluetoothClient::releaseSocket() {
    m_connectionTimer.stop();
    if (!socket) return;

    QBluetoothSocket *old = socket;
    socket = nullptr;
    m_transfer->stop();
    m_transfer->disconnect(this);
    m_transfer = nullptr;
    old->disconnect(this);
    old->abort();
    old->deleteLater();
    emit stateChanged();
}

void BluetoothClient::stopClient() {
    if (discoveryAgent && discoveryAgent->isActive())
        discoveryAgent->stop();
    releaseSocket();
}

void BluetoothClient::sendMessage(const QString &message) {
    sendFrame(BluetoothProtocol::encodeText(message));
}

void BluetoothClient::sendFrame(const QByteArray &frame) {
    if (isConnected() && m_transfer) m_transfer->sendFrame(frame);
}

void BluetoothClient::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    if (error == QBluetoothSocket::SocketError::NoSocketError) return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    const QString errorString = tr("Cannot connect to %1: %2")
            .arg(m_targetAddress, QString::fromLatin1(metaEnum.valueToKey(static_cast<int>(error))));
    qWarning() << "Bluetooth:" << errorString;

    releaseSocket();
    emit socketErrorOccurred(errorString);
}

void BluetoothClient::onSocketDisconnected() {
    releaseSocket();
    emit disconnected();
}

void BluetoothClient::connectedSlot() {
    if (socket) {
        m_connectionTimer.stop();
        addDevice(peerName(), m_targetAddress, false);
        emit connected(peerName());
        emit stateChanged();
    }
}
