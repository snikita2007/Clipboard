#include "bluetooth_reconnect.h"

#include <QBluetoothAddress>
#include <QSettings>

BluetoothReconnect::BluetoothReconnect(QObject *parent) : QObject(parent)
{
    QSettings settings;
    const QBluetoothAddress address(settings.value("Bluetooth/lastDeviceAddress").toString());
    if (!address.isNull()) {
        m_lastAddress = address.toString();
        m_lastName = settings.value("Bluetooth/lastDeviceName", m_lastAddress).toString();
    }

    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout, this, [this]() {
        if (!canConnect()) return;
        m_connecting = true;
        emit connectRequested(m_lastAddress);
    });
}

bool BluetoothReconnect::canConnect() const
{
    return isActive() && m_ready && !m_connected && !m_connecting;
}

void BluetoothReconnect::schedule(int delayMs)
{
    if (canConnect() && !m_retryTimer.isActive())
        m_retryTimer.start(delayMs);
}

void BluetoothReconnect::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    m_paused = false;
    m_retryDelayMs = 5000;
    if (!enabled) {
        m_retryTimer.stop();
        m_ready = false;
    } else {
        schedule(0);
    }
}

void BluetoothReconnect::setReady(bool ready)
{
    if (m_ready == ready) return;
    m_ready = ready;
    if (!ready)
        m_retryTimer.stop();
    else
        schedule(0);
}

void BluetoothReconnect::updateConnectionState(bool connected, bool connecting)
{
    const bool attemptFinished = m_connecting && !connecting && !connected;
    m_connected = connected;
    m_connecting = connecting;
    if (connected || connecting) {
        m_retryTimer.stop();
        if (connected) m_retryDelayMs = 5000;
    } else {
        schedule(m_retryDelayMs);
        if (attemptFinished) m_retryDelayMs = qMin(m_retryDelayMs * 2, 30000);
    }
}

void BluetoothReconnect::rememberDevice(const QString &address, const QString &name)
{
    const QBluetoothAddress peer(address);
    if (peer.isNull()) return;

    m_lastAddress = peer.toString();
    m_lastName = name.isEmpty() ? m_lastAddress : name;
    m_paused = false;
    m_retryDelayMs = 5000;
    QSettings settings;
    settings.setValue("Bluetooth/lastDeviceAddress", m_lastAddress);
    settings.setValue("Bluetooth/lastDeviceName", m_lastName);
    settings.sync();
    emit savedDeviceChanged(m_lastAddress, m_lastName);
}

void BluetoothReconnect::pause()
{
    m_paused = true;
    m_retryTimer.stop();
}
