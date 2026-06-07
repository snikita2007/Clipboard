#include <QApplication>
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
#include "bluetooth_client.h"
#include "switch.h"
#include "QIcon"

#ifndef Q_OS_ANDROID 
#include <QHotkey> 
#endif

#include "WindowEffects.h"
#include "backend.h"

int main(int argc, char *argv[]) {
    #if defined(Q_OS_WIN)
        qputenv("QT_QUICK_CONTROLS_STYLE", "FluentWinUI3");
    #elif defined(Q_OS_MACOS)
        qputenv("QT_QUICK_CONTROLS_STYLE", "macOS");
    #elif defined(Q_OS_ANDROID)
        qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    #else
        qputenv("QT_QUICK_CONTROLS_STYLE", "Imagine");
    #endif

    QApplication app(argc, argv); 

    app.setWindowIcon(QIcon(":/ui/assets/app.png"));

    UdpServer server;
    NetworkScanner scanner;
    BluetoothServer bluetooth;
    BluetoothClient cli;

    NetworkSwitch networkSwitch(&server, &bluetooth, &cli);

    scanner.startScanning(2345);
    
    // server.startServer(1234);    
    
    QBluetoothAddress adapterAddress;
    QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (!devices.isEmpty()) {
        adapterAddress = devices.first().address();
    }
    
    // bluetooth.startServer(adapterAddress);
                 
    // QBluetoothAddress address("F8:5C:7D:2F:20:95");
    // QBluetoothUuid uuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");

    // cli.startDiscovery();

    QObject::connect(&scanner, &NetworkScanner::devicesUpdated, &server, &UdpServer::updateIps);

    QQmlApplicationEngine engine;
    WindowEffects windowEffects;
    Backend backend(&bluetooth,&server,&cli,&networkSwitch);

    // Регистрируем объекты в QML контексте
    // ВАЖНО: используйте маленькую букву "backend", если в QML обращаетесь к ней так
    WindowEffects WindowEffects_linux;

    engine.rootContext()->setContextProperty("networkSwitch", &networkSwitch);
    engine.rootContext()->setContextProperty("networkScanner", &scanner);
    engine.rootContext()->setContextProperty("bluetoothClient", &cli);
    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
    engine.rootContext()->setContextProperty("backend", &backend);

    const QUrl url(QStringLiteral("qrc:/Clipboard/ui/main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    #ifndef Q_OS_ANDROID
    QHotkey hotkey(QKeySequence("Alt+V"), true, &app);
    QObject::connect(&hotkey, &QHotkey::activated, [&engine](){
        if (engine.rootObjects().isEmpty()) return;
        QObject *root = engine.rootObjects().first();
        bool isVisible = root->property("visible").toBool();
        if (isVisible) {
            QMetaObject::invokeMethod(root, "softClose");
        } else {
            QMetaObject::invokeMethod(root, "softOpen");
        }
    });
    #endif

    if (!engine.rootObjects().isEmpty()) {
        QObject *root = engine.rootObjects().first();
        QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

        if (window) {
            windowEffects.applyEffects(window);
        }
    }

    return app.exec();
}
