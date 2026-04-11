#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickWindow>
#include <QTimer>
#include "WindowEffects.h"
#include "backend.h"
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkInterface>
#include <QThread>
#include <QDateTime>
#include <unistd.h>
#include "server.h"
#include <QUdpSocket>
#include <QClipboard>
#include <ip.h>
#include <QThread>
#include <fstream>

int main(int argc, char *argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);
    
    // ExampleThreads threadA("test"); 

    // QStringList IPS;
    
    UdpServer server;
    NetworkScanner scanner;

    scanner.startScanning(1234);

    server.startServer(1234);    

    QObject::connect(&scanner, &NetworkScanner::devicesUpdated, &server, &UdpServer::updateIps);
    
    QQmlApplicationEngine engine;
    WindowEffects windowEffects;
    Backend backend(&server);

    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/Clipboard/ui/main.qml")));

    QObject *root = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

    if (window) {
        windowEffects.applyEffects(window);
        window->show();
    }

    return app.exec();
}

