#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QSet>
#include <QVariantList>

// Finds other ClipboardApp instances on the local network.
// Every few seconds it broadcasts "SEARCH" to each network the device is on.
// A device that gets "SEARCH" answers "HERE"; both sides remember each other.
class NetworkScanner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesUpdated)

public:
    explicit NetworkScanner(QObject *parent = nullptr);
    void startScanning(quint16 port);
    QHostAddress GetIps();
    QVariantList devices() const;

    Q_INVOKABLE void rescan();

signals:
    void devicesUpdated(QVariantList ipList);

private slots:
    void sendDiscoveryRequest();
    void readPendingDatagrams();

private:
    static QHostAddress normalizeIPv4(const QHostAddress &addr);
    static bool isLocalAddress(const QHostAddress &addr);
    QList<QHostAddress> broadcastAddresses() const;
    void addDevice(const QString &ip);

    QUdpSocket *udpSocket;
    QTimer *timer;
    quint16 m_port = 0;
    QSet<QString> foundDevices;
};

#endif
