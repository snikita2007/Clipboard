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

signals:
    void devicesUpdated(QStringList ipList); 

private slots:
    void sendDiscoveryRequest();  
    void readPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    QTimer *timer;
    quint16 m_port;
    QSet<QString> foundDevices;
};

#endif
