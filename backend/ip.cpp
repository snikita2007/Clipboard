#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkInterface>
#include <QThread>
#include <QDateTime>
#include "WindowEffects.h"
#include <unistd.h>
#include "backend.h"
#include "server.h"
#include <QUdpSocket>
#include <QClipboard>
#include <fstream>
#include <examplethreads.h>
#include <ip.h>


using namespace std;

NetworkScanner::NetworkScanner() {
    foundDevices.clear();
}

QString NetworkScanner::getNetworkPrefix() {
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface& iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        
        QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry& entry : entries) {
            QHostAddress ip = entry.ip();

            qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>" << entries << ip;

            if (ip.protocol() == QAbstractSocket::IPv4Protocol && 
                !ip.isLoopback()) {
                
                QString ipString = ip.toString();
                
                if (ipString.startsWith("10.39.")) {

                    QStringList parts = ipString.split('.');
                    parts.removeLast();
                    qDebug() << "Using interface:" << iface.humanReadableName() 
                                << "IP:" << ipString;
                    return parts.join('.');
                }
            }
        }
    }
    
    return "";
}

bool NetworkScanner::pingDevice(const QString& ip) {
    QProcess ping;
    
    #ifdef Q_OS_WIN
        ping.start("ping", QStringList() << "-n" << "1" << "-w" << "500" << ip);
    #else
        ping.start("ping", QStringList() << "-c" << "1" << "-W" << "1" << ip);
    #endif
    
    ping.waitForFinished(1500);
    
    return ping.exitCode() == 0;
}

QString NetworkScanner::getHostname(const QString& ip) {
    QHostAddress address(ip);
    QString hostname = address.toString();
    
    // #ifndef Q_OS_WIN
    //     QProcess arp;
    //     arp.start("arp", QStringList() << "-n" << ip);
    //     arp.waitForFinished(500);
    //     QString output = arp.readAllStandardOutput();
        
    //     QRegularExpression regex("\\s+([0-9a-fA-F:]{17})");
    //     QRegularExpressionMatch match = regex.match(output);
    //     if (match.hasMatch()) {
    //         QString mac = match.captured(1);
    //         return mac;
    //     }
    // #endif
    
    return hostname;
}

QStringList NetworkScanner::scanNetwork(int number) {

    QString networkPrefix = getNetworkPrefix();
    if (networkPrefix.isEmpty()) {
        qDebug() << " Could not determine local network!";
    }
    
    qDebug() << " Scanning IPs from" << networkPrefix + ".1 to" << networkPrefix + ".254";
    
    QDateTime startTime = QDateTime::currentDateTime();
    
    QList<QString> aliveIPs;
    
    for (int i = number - 5; i <= number; i++) {
        QString ip = QString("%1.%2").arg(networkPrefix).arg(i);
        
        if (i % 10 == 0) {
            qDebug() << i;
        }
        
        if (pingDevice(ip)) {
            aliveIPs.append(ip);
            
            QString hostname = getHostname(ip);

            qDebug() << hostname;

            qDebug().quote() << QString("[%1] %2").arg(ip, 15) << hostname;
        }
    }
    
    qDebug() << " devices found:" << aliveIPs.count();
    
    if (aliveIPs.isEmpty()) {
        qDebug() << "  No devices found";
    } else {
        qDebug() << " Found" << aliveIPs.count();
        for (const QString& ip : aliveIPs) {
            qDebug().noquote() << ">>>>>>>>>>>" << ip;
        }
    }

    qDebug() << "delete";

    emit UdpServer::instance().dataReceived(aliveIPs);

    return aliveIPs;
}



