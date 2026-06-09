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

    if (!Selector.isNull()) {
        QString criptonum = "2357";
        QByteArray hash = "H:" + QCryptographicHash::hash(criptonum.toUtf8(), QCryptographicHash::Sha256).toHex();
        udpSocket->writeDatagram(hash, Selector, m_port);
    }
}

void NetworkScanner::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderIp;
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp);
        
        // Преобразуем IPv6-отображение (например, "::ffff:192.168.1.50") в чистый IPv4
        if (senderIp.protocol() == QAbstractSocket::IPv6Protocol) {
            bool ok;
            quint32 ipv4 = senderIp.toIPv4Address(&ok);
            if (ok) senderIp = QHostAddress(ipv4);
        }
        
        QString ip = senderIp.toString();
        
        // Проверяем, чтобы не ловить собственные же запросы SEARCH
        if (QNetworkInterface::allAddresses().contains(senderIp)) continue;

        if (datagram == "SEARCH") {
            udpSocket->writeDatagram("HERE", senderIp, m_port);
        }
        else if (datagram.startsWith("H:")) {
            QByteArray rHash = datagram.mid(2);
            QByteArray myHash = QCryptographicHash::hash("2357", QCryptographicHash::Sha256).toHex();

            if (rHash == myHash) { 
                udpSocket->writeDatagram("CONNECTED_OK", senderIp, m_port);
                qDebug() << "[Scanner] Авторизован IP по хэшу:" << ip;
                Selector = senderIp;  
            }
        }
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

                qDebug() << "[Scanner] Устройство обнаружено! Добавляем в список:" << ip;
                
                // Передаем обновленный список в UI и в синглтон-сервер
                emit devicesUpdated(devicesList);
                UdpServer::instance().updateIps(devicesList);
            }
        }
    }
}

