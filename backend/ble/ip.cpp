#include "ip.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QVariant>
#include <QVariantMap>
#include <QDebug>
#include <QSet>

QHostAddress NetworkScanner::normalizeIPv4(const QHostAddress &addr)
{
    if (addr.protocol() == QAbstractSocket::IPv4Protocol)
        return addr;

    bool ok = false;
    const quint32 ipv4 = addr.toIPv4Address(&ok);
    if (ok)
        return QHostAddress(ipv4);

    return addr;
}

bool NetworkScanner::isLocalAddress(const QHostAddress &addr)
{
    const QHostAddress normalized = normalizeIPv4(addr);
    if (normalized.isLoopback())
        return true;

    for (const QHostAddress &local : QNetworkInterface::allAddresses()) {
        if (normalizeIPv4(local) == normalized)
            return true;
    }
    return false;
}

QList<QHostAddress> NetworkScanner::broadcastAddresses() const
{
    QList<QHostAddress> targets;
    QSet<QString> seen;

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!iface.flags().testFlag(QNetworkInterface::IsUp)
            || iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }

        for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            if (entry.broadcast().isNull())
                continue;

            const QString key = entry.broadcast().toString();
            if (seen.contains(key))
                continue;

            seen.insert(key);
            targets.append(entry.broadcast());
        }
    }

    return targets;
}

void NetworkScanner::addDevice(const QString &ip)
{
    if (ip.isEmpty() || foundDevices.contains(ip))
        return;

    foundDevices.insert(ip);

    QVariantList devicesList;
    for (const QString &savedIp : foundDevices) {
        QVariantMap deviceDictionary;
        deviceDictionary.insert(QStringLiteral("deviceName"), QStringLiteral("TEST_") + savedIp);
        deviceDictionary.insert(QStringLiteral("ip"), savedIp);
        devicesList.append(deviceDictionary);
    }

    qDebug() << "[Scanner] Device added:" << ip << "total:" << foundDevices.size();
    emit devicesUpdated(devicesList);
}

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

    if (!udpSocket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "[Scanner] UDP bind failed on port" << m_port << ":" << udpSocket->errorString();
        return;
    }

    qDebug() << "[Scanner] Listening on UDP port" << m_port;
    timer->start(3000);
    sendDiscoveryRequest();
}

void NetworkScanner::sendDiscoveryRequest() {
    const QByteArray datagram = "SEARCH";

    const qint64 globalSent = udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, m_port);
    if (globalSent < 0) {
        qWarning() << "[Scanner] Global broadcast failed:" << udpSocket->errorString();
    }

    for (const QHostAddress &broadcast : broadcastAddresses()) {
        const qint64 sent = udpSocket->writeDatagram(datagram, broadcast, m_port);
        if (sent < 0) {
            qWarning() << "[Scanner] Subnet broadcast failed for" << broadcast.toString()
                       << ":" << udpSocket->errorString();
        }
    }
}

void NetworkScanner::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderIp;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort);
        senderIp = normalizeIPv4(senderIp);

        if (isLocalAddress(senderIp))
            continue;

        const QString ip = senderIp.toString();

        if (datagram == "SEARCH") {
            udpSocket->writeDatagram("HERE", senderIp, senderPort);
            addDevice(ip);
        } else if (datagram == "HERE") {
            addDevice(ip);
        }
    }
}
