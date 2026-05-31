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

void NetworkSwitch::setMode(TransportMode mode) {
    if (m_currentMode == mode) return;
    m_currentMode = mode;

    if (m_currentMode == TransportMode::Wifi) {
        
        if (m_btServer) m_btServer->stopServer();
        if (m_btClient) m_btClient->stopClient();
        
        if (m_udp) m_udp->startServer(1234); 
    } 
    else if (m_currentMode == TransportMode::Bluetooth) {
        if (m_udp) m_udp->stopServer();

        if (m_btServer) m_btServer->startServer();
        if (m_btClient) m_btClient->startDiscovery();
    }

    emit modeChanged(m_currentMode);
}
