#include "ip.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QDebug>

static constexpr int DISCOVERY_INTERVAL_MS = 5000;

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

QVariantList NetworkScanner::devices() const {
    QVariantList devicesList;
    for (const QString &ip : foundDevices) {
        QVariantMap device;
        device["deviceName"] = ip;
        device["ip"] = ip;
        devicesList.append(device);
    }
    return devicesList;
}

void NetworkScanner::startScanning(quint16 port) {
    m_port = port;
    udpSocket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    // Repeat the search, so devices that start later or change network are found too
    timer->start(DISCOVERY_INTERVAL_MS);
    sendDiscoveryRequest();
}

void NetworkScanner::rescan() {
    sendDiscoveryRequest();
}

QList<QHostAddress> NetworkScanner::broadcastAddresses() const {
    QList<QHostAddress> result;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp) || !flags.testFlag(QNetworkInterface::IsRunning)) continue;
        if (flags.testFlag(QNetworkInterface::IsLoopBack) || !flags.testFlag(QNetworkInterface::CanBroadcast)) continue;

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            const QHostAddress broadcast = entry.broadcast();
            if (!broadcast.isNull() && !result.contains(broadcast)) {
                result.append(broadcast);
            }
        }
    }
    return result;
}

void NetworkScanner::sendDiscoveryRequest() {
    const QByteArray datagram = "SEARCH";

    // Send to the broadcast address of every network (e.g. 192.168.43.255).
    // 255.255.255.255 alone is not enough: the OS sends it to one interface only,
    // which may be a VPN or mobile data instead of the Wi-Fi / hotspot network.
    const QList<QHostAddress> addresses = broadcastAddresses();
    for (const QHostAddress &address : addresses) {
        udpSocket->writeDatagram(datagram, address, m_port);
    }
    udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, m_port);
}

QHostAddress NetworkScanner::normalizeIPv4(const QHostAddress &addr) {
    if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
        bool ok = false;
        const quint32 ipv4 = addr.toIPv4Address(&ok);
        if (ok) return QHostAddress(ipv4);
    }
    return addr;
}

bool NetworkScanner::isLocalAddress(const QHostAddress &addr) {
    const QList<QHostAddress> localAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &local : localAddresses) {
        if (normalizeIPv4(local) == addr) return true;
    }
    return false;
}

void NetworkScanner::addDevice(const QString &ip) {
    if (foundDevices.contains(ip)) return;

    foundDevices.insert(ip);
    qDebug() << "Device found:" << ip;
    emit devicesUpdated(devices());
}

void NetworkScanner::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderIp;
        quint16 senderPort = 0;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort);

        senderIp = normalizeIPv4(senderIp);
        if (senderIp.protocol() != QAbstractSocket::IPv4Protocol) continue;
        if (isLocalAddress(senderIp)) continue;

        if (datagram == "SEARCH") {
            // The device that is searching runs the app too: answer and remember it.
            udpSocket->writeDatagram("HERE", senderIp, senderPort);
            addDevice(senderIp.toString());
        }
        else if (datagram == "HERE") {
            addDevice(senderIp.toString());
        }
    }
}
