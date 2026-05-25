#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    explicit UdpServer(QObject *parent = nullptr);
    bool startServer(quint16 port);
    void sayHello(QVariantMap item); 
    void Recive(QStringList data);
    QVariantList ips;
    static UdpServer& instance();
    void copyToClip(QVariantMap itemData);
    
signals:
    void dataReceived(QVariantList ips);

public slots:
    void updateIps(const QVariantList &newIps);

private slots:
    void Connection();
    
protected:
    QTcpSocket *tcpSocket;
    QTcpServer *m_tcpServer;
    bool isProcessing = false;
    QString lastReceivedHash;
    QStringList Setup(); 

};

#endif
