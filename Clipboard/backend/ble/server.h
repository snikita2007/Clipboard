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

    static UdpServer& instance();

    void Recive(QStringList data);

    QVariantList ips;

signals:
    void dataReceived(QVariantMap data); 
    void serverStopped(); 

public slots:
    bool startServer(quint16 port);
    void stopServer();
    void updateIps(const QVariantList &newIps);
    void sayHello(QVariantMap item);

private slots:
    void Connection(); 
    
protected:
    QTcpServer *m_tcpServer;
    bool isProcessing = false;
    QString lastReceivedHash;
    QStringList Setup(); 
};

#endif 