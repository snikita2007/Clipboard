#include "server.h"
#include <QHostAddress>
#include <QUdpSocket>
#include <QClipboard>
#include <QtWidgets>
#include <QClipboard>
#include <ip.h>
#include "backend.h"
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

void UdpServer::updateIps(const QVariantList &newIps) {
    ips = newIps;
}

void UdpServer::sayHello(QVariantMap item)
{
    qDebug() << "send";
    
    if (ips.isEmpty()) return;
 
    else {
        for (const QVariant &ipadresse : ips) {
            QTcpSocket *socket = new QTcpSocket(this);
                
            QVariantMap deviceDict = ipadresse.toMap();
            
            QString ipa = deviceDict["ip"].toString();

            socket->connectToHost(ipa, 1234);

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


