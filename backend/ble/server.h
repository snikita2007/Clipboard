#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QVariantMap>
#include <QVariantList>
#include <QStringList>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    ~UdpServer();

    bool startServer(quint16 port);
    void sayHello(QVariantMap item); 
    void Recive(QStringList data);
    void stopServer();
    
    static UdpServer& instance();

    QVariantList ips;

signals:
    void dataReceived(QVariantMap data); 
    void serverStopped(); 

public slots:
    void updateIps(const QVariantList &newIps);

private slots:
    void Connection(); 
    
protected:
    QTcpServer *m_tcpServer;
    bool isProcessing = false;
    QString lastReceivedHash;
    QStringList Setup(); 
};

#endif 


