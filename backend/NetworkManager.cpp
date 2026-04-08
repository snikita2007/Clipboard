#include "NetworkManager.h"

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    m_udpSocket = new QUdpSocket(this);

    // 1. Привязываемся строго к IPv4
    // ReuseAddressHint позволяет нескольким копиям приложения работать на одном порту
    bool bound = m_udpSocket->bind(QHostAddress::AnyIPv4, UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!bound) {
        qDebug() << "Ошибка Bind:" << m_udpSocket->errorString();
    }
    if (!m_udpSocket->joinMulticastGroup(GROUP_ADDR)) {
        qDebug() << "Ошибка Join Group:" << m_udpSocket->errorString();
    }
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::processUdpPackets);

    // Таймер для рассылки своего присутствия каждые 5 секунд
    m_discoveryTimer = new QTimer(this);
    connect(m_discoveryTimer, &QTimer::timeout, this, &NetworkManager::sendDiscoveryPacket);
    m_discoveryTimer->start(5000);

    // --- 2. Настройка TCP (Передача) ---
    m_tcpServer = new QTcpServer(this);
    m_tcpServer->listen(QHostAddress::Any, TCP_PORT);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &NetworkManager::onNewTcpConnection);
    
    // Сразу рассылаем пакет при запуске
    sendDiscoveryPacket();
}

void NetworkManager::sendDiscoveryPacket() {
    m_udpSocket->writeDatagram("CLIP_DISCOVERY", GROUP_ADDR, UDP_PORT);
    qDebug() << "Отправка пакета поиска через все интерфейсы...";
}

void NetworkManager::processUdpPackets() {
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        if (datagram.data() == "CLIP_DISCOVERY") {
            QHostAddress senderIp = datagram.senderAddress();
            
            // Если это не наш собственный IP, добавляем в список
            if (!m_knownDevices.contains(senderIp)) {
                m_knownDevices.insert(senderIp);
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
    if (!socket) return;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_0);

    QVariantMap receivedMap;
    in.startTransaction();
    in >> receivedMap;

    if (in.commitTransaction()) {
        emit dataReceived(receivedMap);
    }
}
