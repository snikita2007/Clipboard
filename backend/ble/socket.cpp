#include "server.h"
#include <QThread>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QDebug>

UdpServer& UdpServer::instance() {
    static UdpServer *_instance = nullptr;
    if (!_instance) {
        _instance = new UdpServer();
        
        QThread *netThread = new QThread();
        _instance->moveToThread(netThread); 
        
        connect(netThread, &QThread::finished, _instance, &QObject::deleteLater);
        connect(netThread, &QThread::finished, netThread, &QObject::deleteLater);
        
        netThread->start(); 
    }
    return *_instance;
}

UdpServer::UdpServer(QObject *parent) : QObject(parent), m_tcpServer(nullptr) {}

UdpServer::~UdpServer() {
    if (m_tcpServer) {
        m_tcpServer->close();
    }
}

bool UdpServer::startServer(quint16 port) {
    if (!m_tcpServer) {
        m_tcpServer = new QTcpServer();
        connect(m_tcpServer, &QTcpServer::newConnection, this, &UdpServer::Connection);
    }
    
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "error:" << m_tcpServer->errorString();
        return false;
    }
    return true;
}

void UdpServer::stopServer() {
    if (m_tcpServer && m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
    emit serverStopped(); 
}

void UdpServer::updateIps(const QVariantList &newIps) {
    ips = newIps;
}

void UdpServer::Connection() {
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        QDataStream *stream = new QDataStream(socket);
        stream->setVersion(QDataStream::Qt_6_0);

        connect(socket, &QTcpSocket::readyRead, this, [this, socket, stream]() {
            stream->startTransaction();
            QVariantMap receivedMap; 
            *stream >> receivedMap;

            if (stream->commitTransaction()) {
                emit dataReceived(receivedMap);
                
                socket->disconnectFromHost();
            }
        });

        connect(socket, &QTcpSocket::disconnected, socket, [socket, stream]() {
            delete stream;
            socket->deleteLater();
        });
    }
}

void UdpServer::sayHello(QVariantMap item) {
    qDebug() << "SEND";
    
    QVariantList localIps = this->ips; 
    
    qDebug() << localIps;
    
    if (localIps.isEmpty()) return;

    for (const QVariant &ipadresse : ips) {
        QTcpSocket *socket = new QTcpSocket();
            
        QVariantMap deviceDict = ipadresse.toMap();
        QString ipa = deviceDict["ip"].toString();

        socket->connectToHost(ipa, 1234);

        connect(socket, &QTcpSocket::connected, socket, [socket, item]() {
            QByteArray block;
            QDataStream stream(&block, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_6_0);
            
            QVariantMap sendItem = item;
            QString type = sendItem["type"].toString();
            QString filePath = sendItem["data"].toString();

            if (type == "image") {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly)) {
                    sendItem["img"] = file.readAll(); 
                    file.close();
                }
            }
            else if (type == "file") {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly)) {
                    QFileInfo fileInfo(file);
                    sendItem["file_name"] = fileInfo.fileName();  
                    sendItem["file_bytes"] = file.readAll();   
                    file.close();
                }
            }

            stream << sendItem;
            socket->write(block);
            socket->disconnectFromHost();
        });

        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void UdpServer::Recive(QStringList data) { Q_UNUSED(data); }
QStringList UdpServer::Setup() { return QStringList(); }


