#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

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


class NetworkScanner : public QObject {
    Q_OBJECT

    private: 
        QStringList foundDevices;

        bool pingDevice(const QString& ip);
        QString getNetworkPrefix();
        QString getHostname(const QString& ip);

    public:
        explicit NetworkScanner();
        QStringList scanNetwork(int number);

};

#endif
