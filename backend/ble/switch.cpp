#include <QDebug>
#include "bluetooth.h"
#include "bluetooth_client.h"
#include "server.h"
#include "switch.h"

NetworkSwitch::NetworkSwitch(UdpServer *udp, BluetoothServer *btServer, BluetoothClient *btClient, QObject *parent)
    : QObject(parent), m_udp(udp), m_btServer(btServer), m_btClient(btClient) 
{

    setMode(TransportMode::Wifi);

    connect(m_udp, &UdpServer::serverStopped, [this]() {
        if (m_currentMode == TransportMode::Wifi) {
            qDebug() << "...";
            setMode(TransportMode::Bluetooth);
        }
    });
}

QString NetworkSwitch::toggleMode() {
    if (m_currentMode == TransportMode::Wifi) {
        setMode(TransportMode::Bluetooth);
        emit modeChanged("Bluetooth");
        return "Bluetooth";
    } else {
        setMode(TransportMode::Wifi);
        emit modeChanged("Wifi");
        return "Wifi";
    }
}

QString NetworkSwitch::currentModeString() {
    if (m_currentMode == TransportMode::Bluetooth) return "Bluetooth";
    return "Wifi";
}

void NetworkSwitch::setMode(TransportMode mode) {
    if (m_currentMode == mode) return;
    m_currentMode = mode;

    if (m_currentMode == TransportMode::Wifi) {
        if (m_btServer) m_btServer->stopServer();
        if (m_btClient) m_btClient->stopClient();
        
        if (m_udp) {
            QMetaObject::invokeMethod(m_udp, "startServer", Qt::QueuedConnection, Q_ARG(quint16, 1234));
        } 
    } 
    else if (m_currentMode == TransportMode::Bluetooth) {
        if (m_udp) {
            QMetaObject::invokeMethod(m_udp, "stopServer", Qt::QueuedConnection);
        }

        if (m_btServer) m_btServer->startServer();
        if (m_btClient) m_btClient->startDiscovery();
    }

    emit modeChanged(m_currentMode == TransportMode::Wifi ? "Wifi" : "Bluetooth");
}
