#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    bool startServer(quint16 port);
    void sayHello(); 
    void Recive(QStringList data);
    QStringList ips;
    static UdpServer& instance();

signals:
    void dataReceived(QStringList ips);

private slots:
    void readPendingDatagrams();
    
protected:
    QUdpSocket *udpSocket;
    QStringList Setup(); 

};

#endif
 