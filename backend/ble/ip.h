#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QSet>

class NetworkScanner : public QObject
{
    Q_OBJECT
public:
    explicit NetworkScanner(QObject *parent = nullptr);
    void startScanning(quint16 port); 
    QHostAddress GetIps();
    QVariantList FOUNDDEVICES;
    void generator();
    
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
    quint16 m_port;
    QHostAddress Selector;
    QSet<QString> foundDevices;
};

#endif

