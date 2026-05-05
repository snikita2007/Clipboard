#include "ip.h"
#include <QNetworkInterface>

NetworkScanner::NetworkScanner(QObject *parent) : QObject(parent) {
    udpSocket = new QUdpSocket(this);
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &NetworkScanner::sendDiscoveryRequest);
    connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkScanner::readPendingDatagrams);
}

void NetworkScanner::startScanning(quint16 port) {
    m_port = port;
    udpSocket->bind(QHostAddress::Any, m_port, QUdpSocket::ShareAddress);
    
    timer->start(5000);
    sendDiscoveryRequest();
}

void NetworkScanner::sendDiscoveryRequest() {
    QByteArray datagram = "SEARCH";

    udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, m_port);
}

void NetworkScanner::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderIp;
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp);

        QString ip = senderIp.toString();
        
        if (QNetworkInterface::allAddresses().contains(senderIp)) continue;

        if (datagram == "SEARCH") {
            
            udpSocket->writeDatagram("HERE", senderIp, m_port);
        } 
        else if (datagram == "HERE") {
            
            if (!foundDevices.contains(ip)) {
                foundDevices.insert(ip);
                emit devicesUpdated(foundDevices.values());
                qDebug() << "New device:" << ip << foundDevices.values();

            }
        }
    }
}



