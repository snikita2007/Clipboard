#include "server.h"
#include <QHostAddress>
#include <QUdpSocket>
#include <QClipboard>
#include <QtWidgets>
#include <QClipboard>
#include <ip.h>
#include "backend.h"

// UdpServer& UdpServer::instance() {
//     static UdpServer instance;
//     return instance;
// }

// UdpServer::UdpServer(QObject *parent): 
//     QObject(parent)
// {
//     tcpSocket = new QTcpSocket(this);

//     ips;

//     connect(tcpSocket, &QTcpSocket::readyRead, 
//             this, &UdpServer::readPendingDatagrams);
// }

// bool UdpServer::startServer(quint16 port)
// {
//     bool result = tcpSocket->bind(QHostAddress::Any, port);
    
//     if (result) {
//         qDebug() << "Server started on port" << port;
//     } else {
//         qDebug() << "Failed to bind to port" << port;
//     }
    
//     return result;
// }

// void UdpServer::Recive(QStringList IpAdresses) {
//     ips = IpAdresses;

//     qDebug() << ips;
// }

// QStringList UdpServer::Setup() {   

//     return ips;
// }

// void UdpServer::sayHello(QVariantMap item)
// {
//     QByteArray data;
//     QClipboard *clipboard = QApplication::clipboard();

//     const QMimeData *mimeData = clipboard->mimeData();

//     // QString clipboardText = clipboard->text();
//     // data.append(clipboardText);
//     // data = clipboardText.toUtf8();
    
//     if (item["type"] == "text") {
//         for (QString ip : ips) {
//             qDebug() << ip;   

//             QHostAddress destinationAddress(ip);
//             quint16 destinationPort = 1234;

//             QByteArray datagram;
//             QDataStream stream(&datagram, QIODevice::WriteOnly);
//             stream.setVersion(QDataStream::Qt_6_0); 

//             stream << item;

//             tcpSocket->write(datagram, destinationAddress, destinationPort);

//         }
//         qDebug() << "Sent Text";
//     }

//     if (item["type"] == "image") {
//         for (QString ip : ips) {
//             qDebug() << ip;   
//             QFile file(item["data"].toString());
//             if (file.open(QIODevice::ReadOnly)) {
//                 QByteArray data = file.readAll(); 

//                 QHostAddress destinationAddress(ip);
//                 quint16 destinationPort = 1234;
                
//                 QByteArray datagram;
//                 QDataStream stream(&datagram, QIODevice::WriteOnly);
//                 stream.setVersion(QDataStream::Qt_6_0); 
    
//                 item["img"] = data;
    
//                 stream << item;
    
//                 tcpSocket->writeDatagram(datagram, destinationAddress, destinationPort);
                
//                 file.close();
//             }   


//         }
//         qDebug() << "Sent image";
//     }
// }

// void UdpServer::copyToClip(QVariantMap data)
// {
//     QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
//     QString m_cacheDir = tempPath + "/ClipboardHistory_Cache";

//     QClipboard *clipboard = QGuiApplication::clipboard();
    
//     if (data["type"].toString() == "image") {
//         QByteArray rawData = data["img"].toByteArray();
//         QString hash = QCryptographicHash::hash(rawData, QCryptographicHash::Md5).toHex();
//         QString path = m_cacheDir + QString("/remote_%1.png").arg(hash);
        
//         QFile file(path);
//         if (file.open(QIODevice::WriteOnly)) {
//             file.write(rawData);
//             file.close();
//             clipboard->setImage(QImage(path));
//         }
//     } else {
//         clipboard->setText(data["data"].toString());
//     }
// }


// void UdpServer::readPendingDatagrams()
// {

//     char string[1024];

//     while (tcpSocket->hasPendingDatagrams()) {
        
//         QHostAddress sender;
//         quint16 senderPort;
        
//         Backend backend(nullptr);

//         QByteArray datagram;
//         datagram.resize(tcpSocket->pendingDatagramSize());
//         udpSocket->readDatagram(datagram.data(), datagram.size());

//         QDataStream stream(&datagram, QIODevice::ReadOnly);
//         stream.setVersion(QDataStream::Qt_6_0); 

//         QVariantMap receivedMap;
//         stream >> receivedMap; 

//         if (receivedMap["type"] == "text") {
//             backend.copyToClip(receivedMap);
//         }

//         if (receivedMap["type"] == "img") {
//             qDebug() << receivedMap["img"];
//             copyToClip(receivedMap);
//         }

//         // QByteArray buffer;
//         // buffer.resize(udpSocket->pendingDatagramSize());

//         // QClipboard *clipboard = QApplication::clipboard();
        
//         // udpSocket->readDatagram(buffer.data(), buffer.size(),
//         //                         &sender, &senderPort);
        
//         qDebug() << "Received data-gram from:" << sender.toString() 
//                  << "port:" << senderPort;
//         qDebug() << "Data:" << receivedMap;

//         // if (clipboard -> text() == buffer) {} 
//         // else {copyToClipboard(buffer); }
        
//     }
// }

#include "server.h"
#include <QTcpServer>
#include <QTcpSocket>

UdpServer::UdpServer(QObject *parent): QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &UdpServer::Connection);
}

bool UdpServer::startServer(quint16 port)
{
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "Failed:" << m_tcpServer->errorString();
        return false;
    }
    qDebug() << "Server started on port" << port;
    return true;
}

void UdpServer::copyToClip(QVariantMap data)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString type = data["type"].toString();

    if (type == "image") {
        QByteArray rawData = data["img"].toByteArray();
        if (rawData.isEmpty()) return;

        QString newHash = QCryptographicHash::hash(rawData, QCryptographicHash::Md5).toHex();
        
        QImage currentImg = clipboard->image();
        QByteArray currentData;
        QBuffer buffer(&currentData);
        buffer.open(QIODevice::WriteOnly);
        currentImg.save(&buffer, "JPG"); 
        QString currentHash = QCryptographicHash::hash(currentData, QCryptographicHash::Md5).toHex();

        if (newHash == currentHash) {
            qDebug() << "Not upload";
            return;
        }

        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString m_cacheDir = tempPath + "/ClipboardHistory_Cache";
        QString path = m_cacheDir + QString("/remote_%1.png").arg(newHash);
        
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(rawData);
            file.close();
            clipboard->setImage(QImage(path));
            qDebug() << "Added to buffer";
        }
    } 
    else if (type == "text") {
        QString newText = data["data"].toString();
        
        if (clipboard->text() == newText) {
            qDebug() << "Not upload";
            return;
        }

        clipboard->setText(newText);
        qDebug() << "Added to buffer";
    }
}

void UdpServer::Connection()
{
    // QTcpSocket *clientSocket = tcpServer->nextPendingConnection();

    // QDataStream *stream = new QDataStream(clientSocket);
    // stream->setVersion(QDataStream::Qt_6_0);

    // connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket, stream]() {

    //     stream->startTransaction();

    //     QVariantMap receivedMap;
    //     *stream >> receivedMap;

    //     if (stream->commitTransaction()) {
    //         if (receivedMap.isEmpty()) {
    //             qDebug() << "wait";
    //             return;
    //         }
                
    //         this->copyToClip(receivedMap);
    //         qDebug() << "received"; 
    //         clientSocket->disconnectFromHost();

    //     } else {
    //         qDebug() << "waiting";
    //     }
    // });

    // connect(clientSocket, &QTcpSocket::disconnected, this, [clientSocket, stream]() {
    //     delete stream;     
    //     clientSocket->deleteLater(); 
    // });

    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();

        QDataStream *stream = new QDataStream(socket);
        stream->setVersion(QDataStream::Qt_6_0);

        connect(socket, &QTcpSocket::readyRead, this, [this, socket, stream]() {

            stream->startTransaction();
            QVariantMap receivedMap;
            *stream >> receivedMap;

            if (stream->commitTransaction()) {
                QString currentHash = receivedMap["hash"].toString();

                if (!currentHash.isEmpty() && currentHash == this->lastReceivedHash) {
                    socket->disconnectFromHost();
                    return;
                }

                this->lastReceivedHash = currentHash;

                qDebug() << "Data recived";
                this->copyToClip(receivedMap);
                
                socket->disconnectFromHost();
            }
        });

        connect(socket, &QTcpSocket::disconnected, this, [socket, stream]() {
            delete stream;
            socket->deleteLater();
        });
    }
}

void UdpServer::updateIps(const QStringList &newIps) {
    ips = newIps;
}

void UdpServer::sayHello(QVariantMap item)
{
    if (ips.isEmpty()) return;

    else {
        for (const QString &ip : ips) {
            QTcpSocket *socket = new QTcpSocket(this);
            
            socket->connectToHost(ip, 1234);

            connect(socket, &QTcpSocket::connected, this, [socket, item]() {
                QByteArray block;
                QDataStream stream(&block, QIODevice::WriteOnly);
                stream.setVersion(QDataStream::Qt_6_0);
                
                QVariantMap sendItem = item;
                
                if (sendItem["type"] == "image") {
                    QFile file(sendItem["data"].toString());
                    if (file.open(QIODevice::ReadOnly)) {
                        sendItem["img"] = file.readAll();
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
}
