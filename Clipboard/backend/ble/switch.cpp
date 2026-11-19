#include "switch.h"

#include <QCoreApplication>
#include <QPermissions>
#include <QSettings>

NetworkSwitch::NetworkSwitch(UdpServer *udp, BluetoothServer *btServer, BluetoothClient *btClient, QObject *parent)
    : QObject(parent), m_udp(udp), m_btServer(btServer), m_btClient(btClient)
{
    QSettings settings;
    m_bluetoothOnStartup = settings.value("ApplicationSettings/bluetoothOnStartup", true).toBool();
    const int savedTransport = settings.value("ApplicationSettings/transportMode", 0).toInt();
    m_currentMode = (m_bluetoothOnStartup || savedTransport == 1) ? Bluetooth : Wifi;

    m_startRetryTimer.setSingleShot(true);
    m_startRetryTimer.setInterval(5000);
    connect(&m_startRetryTimer, &QTimer::timeout, this, &NetworkSwitch::startBluetoothWithPermission);

    m_powerOnTimer.setSingleShot(true);
    m_powerOnTimer.setInterval(15000);
    connect(&m_powerOnTimer, &QTimer::timeout, this, [this]() {
        updateBluetoothStatus(tr("Bluetooth is still off. Enable it in system settings; connection will resume automatically."));
    });

    if (m_btServer) {
        connect(m_btServer, &BluetoothServer::transferError, this, &NetworkSwitch::reportBluetoothTransfer);
        connect(m_btServer, &BluetoothServer::transferFinished, this, [this]() {
            reportBluetoothTransfer(tr("Clipboard sent over Bluetooth."));
        });
        connect(m_btServer, &BluetoothServer::peerConnected, this,
                [this](const QString &address, const QString &name) {
            m_reconnect.rememberDevice(address, name);
            updateConnectionState();
        });
        connect(m_btServer, QOverload<const QString &>::of(&BluetoothServer::clientDisconnected),
                this, [this](const QString &) { updateConnectionState(); });
        connect(m_btServer, &BluetoothServer::errorOccurred, this, [this](const QString &error) {
            updateBluetoothStatus(error);
        });
    }
    if (m_btClient) {
        connect(m_btClient, &BluetoothClient::transferError, this, &NetworkSwitch::reportBluetoothTransfer);
        connect(m_btClient, &BluetoothClient::transferFinished, this, [this]() {
            reportBluetoothTransfer(tr("Clipboard sent over Bluetooth."));
        });
        m_btClient->setSavedDevice(m_reconnect.lastAddress(), m_reconnect.lastName());
        connect(&m_reconnect, &BluetoothReconnect::savedDeviceChanged, m_btClient, &BluetoothClient::setSavedDevice);
        connect(m_btClient, &BluetoothClient::connected, this, [this](const QString &name) {
            m_reconnect.rememberDevice(m_btClient->connectedAddress(), name);
            updateConnectionState();
        });
        connect(m_btClient, &BluetoothClient::connectionRequested, &m_reconnect, &BluetoothReconnect::pause);
        connect(m_btClient, &BluetoothClient::disconnectRequested, &m_reconnect, &BluetoothReconnect::pause);
        connect(m_btClient, &BluetoothClient::stateChanged, this, &NetworkSwitch::updateConnectionState);
        connect(m_btClient, &BluetoothClient::disconnected, this, &NetworkSwitch::updateConnectionState);
        connect(m_btClient, &BluetoothClient::discoveringChanged, this, &NetworkSwitch::updateBluetoothReadiness);
        connect(m_btClient, &BluetoothClient::socketErrorOccurred, this, [this](const QString &error) {
            updateConnectionState();
            updateBluetoothStatus(error);
        });
    }
    connect(&m_reconnect, &BluetoothReconnect::connectRequested, this, [this](const QString &address) {
        if (!m_localDevice || !m_localDevice->isValid()
                || m_localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
            m_reconnect.setReady(false);
            updateConnectionState();
            startBluetoothWithPermission();
            return;
        }
        if (m_btClient) m_btClient->reconnectToDevice(address);
    });
    updateBluetoothStatus();
}

void NetworkSwitch::start()
{
    if (m_started) return;
    m_started = true;
    applyMode();
}

void NetworkSwitch::setBluetoothOnStartup(bool enabled)
{
    if (m_bluetoothOnStartup == enabled) return;
    m_bluetoothOnStartup = enabled;
    QSettings().setValue("ApplicationSettings/bluetoothOnStartup", enabled);
    emit bluetoothOnStartupChanged();
}

QString NetworkSwitch::toggleMode()
{
    setMode(m_currentMode == Wifi ? Bluetooth : Wifi);
    return currentModeString();
}

QString NetworkSwitch::currentModeString()
{
    return m_currentMode == Bluetooth ? "Bluetooth" : "Wifi";
}

void NetworkSwitch::setTransport(int index)
{
    setMode(index == 1 ? Bluetooth : Wifi);
}

void NetworkSwitch::refreshBluetooth()
{
    if (!m_started || m_currentMode != Bluetooth) return;
    m_discoveryRequested = true;
    if (!m_powerOnTimer.isActive()) m_powerOnRequested = false;
    startBluetoothWithPermission();
}

bool NetworkSwitch::bluetoothConnected() const
{
    return m_currentMode == Bluetooth
        && ((m_btClient && m_btClient->isConnected()) || (m_btServer && m_btServer->hasActiveConnections()));
}

void NetworkSwitch::setMode(TransportMode mode)
{
    if (mode != Bluetooth && mode != Wifi) return;
    // Save explicit choices even when startup already selected the same mode.
    QSettings().setValue("ApplicationSettings/transportMode", mode == Bluetooth ? 1 : 0);
    if (m_currentMode == mode) return;
    m_currentMode = mode;
    reportBluetoothTransfer(QString());
    if (m_started) applyMode();
    else updateBluetoothStatus();
    emit modeChanged(currentModeString());
}

void NetworkSwitch::reportBluetoothTransfer(const QString &status)
{
    if (m_bluetoothTransferStatus == status) return;
    m_bluetoothTransferStatus = status;
    emit bluetoothTransferStatusChanged();
}

void NetworkSwitch::applyMode()
{
    m_startRetryTimer.stop();
    m_powerOnTimer.stop();
    m_powerOnRequested = false;
    m_reconnect.setEnabled(m_currentMode == Bluetooth);

    if (m_currentMode == Wifi) {
        if (m_btServer) m_btServer->stopServer();
        if (m_btClient) m_btClient->stopClient();
        if (m_udp)
            QMetaObject::invokeMethod(m_udp, "startServer", Qt::QueuedConnection, Q_ARG(quint16, 1234));
        updateConnectionState();
    } else {
        if (m_udp)
            QMetaObject::invokeMethod(m_udp, "stopServer", Qt::QueuedConnection);
        m_discoveryRequested = m_reconnect.lastAddress().isEmpty();
        updateConnectionState();
        startBluetoothWithPermission();
    }
}

void NetworkSwitch::startBluetoothWithPermission()
{
    if (!m_started || m_currentMode != Bluetooth || m_permissionPending) return;
#if QT_CONFIG(permissions)
    QBluetoothPermission permission;
    permission.setCommunicationModes(QBluetoothPermission::Access | QBluetoothPermission::Advertise);
    switch (qApp->checkPermission(permission)) {
    case Qt::PermissionStatus::Granted:
        startBluetooth();
        return;
    case Qt::PermissionStatus::Undetermined:
        m_permissionPending = true;
        updateBluetoothStatus(tr("Waiting for Bluetooth permission..."));
        qApp->requestPermission(permission, this, [this](const QPermission &result) {
            m_permissionPending = false;
            if (m_currentMode != Bluetooth) return;
            if (result.status() == Qt::PermissionStatus::Granted) {
                startBluetooth();
            } else {
                m_reconnect.setReady(false);
                updateBluetoothStatus(tr("Bluetooth permission was denied. Allow Bluetooth access for the app in system settings."));
            }
        });
        return;
    case Qt::PermissionStatus::Denied:
        m_reconnect.setReady(false);
        updateBluetoothStatus(tr("Bluetooth permission was denied. Allow Bluetooth access for the app in system settings."));
        return;
    }
#else
    startBluetooth();
#endif
}

void NetworkSwitch::startBluetooth()
{
    if (!m_started || m_currentMode != Bluetooth) return;
    m_startRetryTimer.stop();

    if (!m_localDevice || !m_localDevice->isValid()) {
        m_powerOnTimer.stop();
        m_powerOnRequested = false;
        delete m_localDevice;
        m_localDevice = new QBluetoothLocalDevice(this);
        connect(m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged, this,
                [this](QBluetoothLocalDevice::HostMode mode) {
            if (m_currentMode != Bluetooth) return;
            if (mode == QBluetoothLocalDevice::HostPoweredOff) {
                m_reconnect.setReady(false);
                m_startRetryTimer.stop();
                // Do not fight a user turning the system adapter off.
                m_powerOnRequested = true;
                if (m_btClient) m_btClient->stopClient();
                if (m_btServer) m_btServer->stopServer();
                updateConnectionState();
                updateBluetoothStatus(tr("Bluetooth is off. Enable it to resume the connection."));
            } else {
                m_powerOnTimer.stop();
                m_powerOnRequested = false;
                startBluetooth();
            }
        });
    }
    if (!m_localDevice->isValid()) {
        m_reconnect.setReady(false);
        updateBluetoothStatus(tr("Waiting for a Bluetooth adapter..."));
        m_startRetryTimer.start();
        return;
    }
    if (m_localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        m_reconnect.setReady(false);
        if (!m_powerOnRequested) {
            m_powerOnRequested = true;
            updateBluetoothStatus(tr("Turning on Bluetooth. Confirm the system prompt if one appears."));
            m_powerOnTimer.start();
            m_localDevice->powerOn();
        }
        return;
    }

    m_powerOnTimer.stop();
    const bool serverOk = m_btServer && m_btServer->startServer();
    if (!serverOk && m_btServer) m_startRetryTimer.start();
    if (m_discoveryRequested && m_btClient) {
        m_discoveryRequested = false;
        m_btClient->startDiscovery();
    }
    updateConnectionState();
    updateBluetoothReadiness();
    if (!serverOk && m_btServer)
        updateBluetoothStatus(tr("Cannot start the Bluetooth service. Retrying..."));
}

void NetworkSwitch::updateConnectionState()
{
    m_reconnect.updateConnectionState(bluetoothConnected(), m_btClient && m_btClient->isConnecting());
    updateBluetoothStatus();
}

void NetworkSwitch::updateBluetoothReadiness()
{
    m_reconnect.setReady(m_started && m_currentMode == Bluetooth && m_btClient
        && m_localDevice && m_localDevice->isValid()
        && m_localDevice->hostMode() != QBluetoothLocalDevice::HostPoweredOff
        && !m_btClient->isDiscovering());
}

void NetworkSwitch::updateBluetoothStatus(const QString &error)
{
    QString status;
    if (m_currentMode != Bluetooth) {
        status = tr("Bluetooth is off. Sync goes over Wi-Fi.");
    } else if (!error.isEmpty()) {
        status = error;
    } else if (m_btClient && m_btClient->isConnected()) {
        status = tr("Connected to %1").arg(m_btClient->peerName());
    } else if (m_btServer && m_btServer->hasActiveConnections()) {
        status = tr("Connected to %1").arg(m_btServer->connectedNames().join(", "));
    } else if (m_btClient && m_btClient->isConnecting()) {
        status = tr("Connecting...");
    } else if (m_btServer && m_btServer->isRunning()) {
        status = m_reconnect.isActive()
            ? tr("Waiting to reconnect to %1...").arg(m_reconnect.lastName())
            : tr("Not connected. Choose a device below or connect from the other device.");
    } else {
        status = tr("Starting Bluetooth...");
    }
    m_bluetoothStatus = status;
    emit bluetoothStatusChanged();
}
