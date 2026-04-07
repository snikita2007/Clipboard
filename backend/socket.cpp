#include "server.h"
#include <QHostAddress>
#include <QUdpSocket>
#include <QClipboard>
#include <QtWidgets>
#include <QClipboard>
#include <ip.h>

UdpServer& UdpServer::instance() {
    static UdpServer instance;
    return instance;
}

UdpServer::UdpServer(QObject *parent): 
    QObject(parent)
{
    udpSocket = new QUdpSocket(this);

    ips;

    connect(udpSocket, &QUdpSocket::readyRead, 
            this, &UdpServer::readPendingDatagrams);
}

bool UdpServer::startServer(quint16 port)
{
    bool result = udpSocket->bind(QHostAddress::Any, port);
    
    if (result) {
        qDebug() << "Server started on port" << port;
    } else {
        qDebug() << "Failed to bind to port" << port;
    }
    
    return result;
}

void UdpServer::Recive(QStringList IpAdresses) {
    ips = IpAdresses;

    qDebug() << ips;
}

QStringList UdpServer::Setup() {   

    return ips;
}

void UdpServer::sayHello()
{
    QByteArray data;
    QClipboard *clipboard = QApplication::clipboard();

    const QMimeData *mimeData = clipboard->mimeData();
    QVariantMap item;

    // if (mimeData->hasImage()) {
        
    //     QImage image = qvariant_cast<QImage>(mimeData->imageData());
        
    //     if (!image.isNull()) {
    //         NULL;
    //     }
    // }

    QString clipboardText = clipboard->text();

    // data.append(clipboardText);

    data = clipboardText.toUtf8(); 
    
    qDebug() << ips;

    for (QString ip : ips) {
        qDebug() << ip;   
    
        QHostAddress destinationAddress(ip);
        quint16 destinationPort = 1234;

        udpSocket -> writeDatagram(data, destinationAddress, destinationPort);
    }

    qDebug() << "Sent message";
}

void copyToClipboard(const QString &text) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard -> setText(text); 
}

void UdpServer::readPendingDatagrams()
{
    char string[1024];

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());

        QClipboard *clipboard = QApplication::clipboard();
        
        QHostAddress sender;
        quint16 senderPort;
        
        udpSocket->readDatagram(buffer.data(), buffer.size(),
                                &sender, &senderPort);
        
        qDebug() << "Received data-gram from:" << sender.toString() 
                 << "port:" << senderPort;
        qDebug() << "Data:" << buffer;

        if (clipboard -> text() == buffer) {} 
        else {copyToClipboard(buffer); }
        
    }
}
