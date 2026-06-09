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
    // ИСПРАВЛЕНИЕ: Метод занимается только широковещательным поиском, 
    // авторизация перенесена в цепочку обработки пакетов.
    udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, m_port); 
}

void NetworkScanner::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderIp;
        quint16 senderPort; // ИСПРАВЛЕНИЕ: Запоминаем точный порт отправителя для обратного ответа
        
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort);
        
        if (senderIp.protocol() == QAbstractSocket::IPv6Protocol) {
            bool ok;
            quint32 ipv4 = senderIp.toIPv4Address(&ok);
            if (ok) senderIp = QHostAddress(ipv4);
        }
        
        QString ip = senderIp.toString();
        
        // ВНИМАНИЕ: Если вы запускаете два экземпляра программы на ОДНОМ компьютере для тестов,
        // временно закомментируйте строку ниже, иначе они будут игнорировать друг друга.
        if (QNetworkInterface::allAddresses().contains(senderIp)) continue;

        // ЭТАП 1: Получен широковещательный запрос от другого устройства
        if (datagram == "SEARCH") {
            udpSocket->writeDatagram("HERE", senderIp, senderPort);
        }
        // ЭТАП 2: Другое устройство ответило, что оно в сети. Инициируем проверку хэша.
        else if (datagram == "HERE") {
            QString criptonum = "2357";
            QByteArray hash = "H:" + QCryptographicHash::hash(criptonum.toUtf8(), QCryptographicHash::Sha256).toHex();
            udpSocket->writeDatagram(hash, senderIp, senderPort);
        }
        // ЭТАП 3: Нам прислали хэш. Проверяем его валидность.
        else if (datagram.startsWith("H:")) {
            QByteArray rHash = datagram.mid(2);
            QByteArray myHash = QCryptographicHash::hash("2357", QCryptographicHash::Sha256).toHex();

            if (rHash == myHash) { 
                udpSocket->writeDatagram("CONNECTED_OK", senderIp, senderPort);
                qDebug() << "[Scanner] Авторизован удаленный IP по хэшу:" << ip;
                Selector = senderIp;  
            }
        }
        // ЭТАП 4: Наш хэш успешно прошел проверку на удаленной стороне. Добавляем устройство.
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

                qDebug() << "[Scanner] Устройство подтвердило авторизацию! Добавляем в список:" << ip;
                
                emit devicesUpdated(devicesList);
                UdpServer::instance().updateIps(devicesList);
            }
        }
    }
}