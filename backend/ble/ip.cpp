#include "ip.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QCryptographicHash> 
#include <QNetworkDatagram>
#include <QDebug>
#include "server.h"
#include <QThread>

NetworkScanner::NetworkScanner(QObject *parent) : QObject(parent) {
    udpSocket = new QUdpSocket(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NetworkScanner::sendDiscoveryRequest);
    connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkScanner::readPendingDatagrams);
}

QHostAddress NetworkScanner::GetIps() {
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces) {
        if (!iface.flags().testFlag(QNetworkInterface::IsUp) || iface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;

        if (iface.type() != QNetworkInterface::Wifi && iface.type() != QNetworkInterface::Ethernet) continue;

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.broadcast().isNull()) {
                return entry.ip(); 
            }
        }
    }
    return QHostAddress();
}

void NetworkScanner::startScanning(quint16 port) {
    m_port = port;
    
    udpSocket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    
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
        quint16 senderPort; 
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort);
        
        // Превращаем IPv6-вид (::ffff:10.154.98.125) в чистый IPv4
        if (senderIp.protocol() == QAbstractSocket::IPv6Protocol) {
            bool ok;
            quint32 ipv4 = senderIp.toIPv4Address(&ok);
            if (ok) senderIp = QHostAddress(ipv4);
        }
        
        QString ip = senderIp.toString();
        
        // Отсекаем свои собственные пакеты
        if (QNetworkInterface::allAddresses().contains(senderIp)) continue;

        // ШАГ 1: Кто-то прислал SEARCH — отвечаем ему "HERE"
        if (datagram == "SEARCH") {
            udpSocket->writeDatagram("HERE", senderIp, senderPort);
        }
        
        // ШАГ 2: Нам ответили "HERE". Сразу добавляем устройство без лишних проверок!
        else if (datagram == "HERE") {
            if (!foundDevices.contains(ip)) {
                foundDevices.insert(ip);

                QVariantList devicesList;
                for (const QString &savedIp : foundDevices) {
                    QVariantMap deviceDictionary; 
                    deviceDictionary["deviceName"] = "TEST_" + savedIp; 
                    deviceDictionary["ip"] = savedIp;  
                    devicesList.append(deviceDictionary);
                }

                qDebug() << "[Scanner] УСПЕХ! По сигналу HERE обнаружено устройство:" << ip;
                qDebug() << "[Scanner] Текущий список для отправки буфера:" << devicesList;

                emit devicesUpdated(devicesList);

                UdpServer::instance().updateIps(devicesList);
            }
        }
    }
}
