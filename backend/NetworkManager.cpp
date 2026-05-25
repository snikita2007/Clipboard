#include "NetworkManager.h"

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);

    // Привязываемся к порту
    m_udpSocket->bind(QHostAddress::AnyIPv4, UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

<<<<<<< HEAD
=======

>>>>>>> vibe-branch
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::processUdpPackets);

    m_discoveryTimer = new QTimer(this);
    connect(m_discoveryTimer, &QTimer::timeout, this, &NetworkManager::sendDiscoveryPacket);
    m_discoveryTimer->start(2000);

    // --- 2. Настройка TCP (Передача) ---
    m_tcpServer = new QTcpServer(this);
    m_tcpServer->listen(QHostAddress::Any, TCP_PORT);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::onNewTcpConnection);
    
    // Сразу рассылаем пакет при запуске
    sendDiscoveryPacket();
}

void NetworkManager::sendDiscoveryPacket() {
    m_udpSocket->writeDatagram("CLIP_DISCOVERY", GROUP_ADDR, UDP_PORT);
    qDebug() << "Sending packages (all ports)...";
}

void NetworkManager::processUdpPackets() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QHostAddress senderIp = datagram.senderAddress();

        // Проверяем, не наш ли это адрес
        bool isMe = false;
        const QList<QHostAddress> allAddresses = QNetworkInterface::allAddresses();
        for (const QHostAddress &address : allAddresses) {
            if (senderIp == address) {
                isMe = true;
                break;
            }
        }

        if (isMe) continue; // Игнорируем пакеты от самого себя

        if (datagram.data() == "CLIP_DISCOVERY") {
            if (!m_knownDevices.contains(senderIp)) {
                m_knownDevices.insert(senderIp);
                qDebug() << "Device spoted:" << senderIp.toString();
                emit deviceFound(senderIp.toString());
            }
        }
    }
}


void NetworkManager::broadcastClipboard(const QVariantMap &data) {
    for (const QHostAddress &ip : m_knownDevices) {
        QTcpSocket *socket = new QTcpSocket(this);
        
        connect(socket, &QTcpSocket::connected, [socket, data]() {
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_6_0);
            out << data;
            socket->write(block);
            socket->disconnectFromHost();
        });

        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        socket->connectToHost(ip, TCP_PORT);
    }
}

void NetworkManager::onNewTcpConnection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::readIncomingData);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void NetworkManager::readIncomingData() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_0);

    QVariantMap receivedMap;
    
    // ТРАНЗАКЦИЯ ГАРАНТИРУЕТ, ЧТО МЫ НЕ НАЧНЕМ ЧИТАТЬ, ПОКА ВСЕ БАЙТЫ НЕ ПРИДУТ
    in.startTransaction();
    in >> receivedMap;

    if (in.commitTransaction()) {
        qDebug() << "ДАННЫЕ ПОЛУЧЕНЫ ПОЛНОСТЬЮ!";
        emit dataReceived(receivedMap);
    } else {
        // Если данных мало, Qt просто подождет следующего readyRead
        qDebug() << "Ждем дозагрузки данных... Текущий буфер:" << socket->bytesAvailable();
    }
}
