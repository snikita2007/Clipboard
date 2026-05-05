#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickWindow>
#include <QTimer>
#include "WindowEffects.h"
#include "backend.h"
#include <QProcess>
#include <unistd.h>
#include "server.h"
#include <QUdpSocket>
#include <ip.h>
#include <QBluetoothServer>    
#include <QBluetoothSocket>           
#include <QBluetoothServiceInfo>      
#include <QBluetoothLocalDevice>      
#include <QBluetoothAddress>          
#include <QBluetoothHostInfo>         
#include <QBluetoothUuid>             
#include "bluetooth.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);
    
    UdpServer server;
    NetworkScanner scanner;

    scanner.startScanning(1234);

    server.startServer(1234);    

    BluetoothServer bluetooth;

    QBluetoothAddress adapterAddress;
    QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (!devices.isEmpty()) {
        adapterAddress = devices.first().address();
    }

    bluetooth.startServer(adapterAddress);

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

