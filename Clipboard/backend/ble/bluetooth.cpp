#include "bluetooth.h"
#include <QBluetoothServer>
#include <QBluetoothSocket>

using namespace Qt::StringLiterals;

static constexpr auto serviceUuid = "e8e10f95-1a70-4b27-9ccf-02010264e9c8"_L1;

BluetoothServer::BluetoothServer(QObject *parent)
    :   QObject(parent)
{}

BluetoothServer::~BluetoothServer()
{
    stopServer();
}

bool BluetoothServer::hasActiveConnections() {
    return !clientSockets.isEmpty();
}

QStringList BluetoothServer::connectedNames() const {
    QStringList names;
    for (QBluetoothSocket *socket : clientSockets) {
        const QString name = clientNames.value(socket);
        names.append(name.isEmpty() ? socket->peerAddress().toString() : name);
    }
    return names;
}

bool BluetoothServer::startServer(const QBluetoothAddress& localAdapter)
{
    if (rfcommServer)
        return true;

    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, &QBluetoothServer::newConnection,
            this, QOverload<>::of(&BluetoothServer::clientConnected));
    connect(rfcommServer, &QBluetoothServer::errorOccurred, this, [this](QBluetoothServer::Error error) {
        emit errorOccurred(tr("Bluetooth server error %1").arg(int(error)));
    });

    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Bluetooth server: cannot listen on" << localAdapter.toString();
        delete rfcommServer;
        rfcommServer = nullptr;
        emit errorOccurred(tr("Cannot start Bluetooth server. Is Bluetooth turned on?"));
        return false;
    }

    QBluetoothServiceInfo::Sequence profileSequence;
    QBluetoothServiceInfo::Sequence classId;
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    classId << QVariant::fromValue(quint16(0x100));
    profileSequence.append(QVariant::fromValue(classId));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                             profileSequence);

    classId.clear();
    classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Clipboard Sync"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("Clipboard sync between devices"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("ClipboardApp"));

    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

    const auto groupUuid = QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup);
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(groupUuid);
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
             << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);

    if (!serviceInfo.registerService(localAdapter)) {
        qWarning() << "Bluetooth server: cannot register service";
        stopServer();
        emit errorOccurred(tr("Cannot advertise the Clipboard Bluetooth service."));
        return false;
    }

    qDebug() << "Bluetooth server started";
    return true;
}

void BluetoothServer::stopServer()
{
    if (serviceInfo.isRegistered())
        serviceInfo.unregisterService();

    // Disconnect first, so deleting the sockets does not call our slots
    const QList<QBluetoothSocket *> sockets = clientSockets;
    clientSockets.clear();
    clientNames.clear();
    for (QBluetoothSocket *socket : sockets) {
        if (auto *transfer = transfers.take(socket)) {
            transfer->stop();
            transfer->disconnect(this);
        }
        socket->disconnect(this);
        socket->abort();
        socket->deleteLater();
    }

    delete rfcommServer;
    rfcommServer = nullptr;
}

void BluetoothServer::sendMessage(const QString &message)
{
    sendFrame(BluetoothProtocol::encodeText(message));
}

void BluetoothServer::sendFrame(const QByteArray &frame)
{
    const auto sockets = clientSockets;
    for (QBluetoothSocket *socket : sockets) {
        if (auto *transfer = transfers.value(socket)) transfer->sendFrame(frame);
    }
}

void BluetoothServer::clientConnected()
{
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if (!socket)
        return;

    auto *transfer = new BluetoothTransfer(socket, socket);
    transfers[socket] = transfer;
    connect(transfer, &BluetoothTransfer::itemReceived, this, [this, socket](const QVariantMap &item) {
        emit dataReceived(item);
        if (item.value("type").toString() == "text")
            emit messageReceived(clientNames.value(socket), item.value("data").toString());
    });
    connect(transfer, &BluetoothTransfer::transferError, this, &BluetoothServer::transferError);
    connect(transfer, &BluetoothTransfer::finished, this, &BluetoothServer::transferFinished);
    connect(socket, &QBluetoothSocket::disconnected, this, QOverload<>::of(&BluetoothServer::clientDisconnected));
    clientSockets.append(socket);
    const QString name = socket->peerName().isEmpty() ? socket->peerAddress().toString() : socket->peerName();
    clientNames[socket] = name;
    emit peerConnected(socket->peerAddress().toString(), name);
    emit clientConnected(name);
}

void BluetoothServer::clientDisconnected()
{
    QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket || !clientSockets.contains(socket))
        return;

    const QString name = clientNames.value(socket);
    clientSockets.removeOne(socket);
    clientNames.remove(socket);
    if (auto *transfer = transfers.take(socket)) {
        transfer->stop();
        transfer->disconnect(this);
    }
    socket->deleteLater();

    emit clientDisconnected(name);
}

