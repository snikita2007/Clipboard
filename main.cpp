#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
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

int main(int argc, char *argv[]) {
    #if defined(Q_OS_WIN)
        qputenv("QT_QUICK_CONTROLS_STYLE", "FluentWinUI3");
    #elif defined(Q_OS_MACOS)
        qputenv("QT_QUICK_CONTROLS_STYLE", "macOS");
    #elif defined(Q_OS_ANDROID)
        qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
        qputenv("QT_QUICK_CONTROLS_MATERIAL_ACCENT", "#333333");
    #else
        qputenv("QT_QUICK_CONTROLS_STYLE", "Imagine");
    #endif

    QApplication app(argc, argv); 

    app.setOrganizationName("KNGC");
    app.setOrganizationDomain("kngc.com");
    app.setApplicationName("ClipboardApp");
    app.setWindowIcon(QIcon(":/ui/assets/app.png"));

    // ИСПРАВЛЕНО: Вместо локального объекта на стеке берем ссылку на наш ЕДИНЫЙ фоновый поток!
    UdpServer *serverInstance = &UdpServer::instance();

    NetworkScanner scanner;
    BluetoothServer bluetooth;
    BluetoothClient cli;

    // Передаем синглтон-сервер в коммутатор
    NetworkSwitch networkSwitch(serverInstance, &bluetooth, &cli);

    // Запускаем сканирование сети
    scanner.startScanning(1234);
    
    QBluetoothAddress adapterAddress;
    QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (!devices.isEmpty()) {
        adapterAddress = devices.first().address();
    }
    
    // ИСПРАВЛЕНО: Связываем сигнал сканера НАПРЯМУЮ с фоновым синглтоном сервера.
    // Теперь список IPs гарантированно попадет в сетевой поток отправки пакетов!
    QObject::connect(
        &scanner,
        &NetworkScanner::devicesUpdated,
        serverInstance,
        &UdpServer::updateIps,
        Qt::QueuedConnection);

    QQmlApplicationEngine engine;
    WindowEffects windowEffects;
    
    // Передаем синглтон-сервер в бэкэнд
    Backend backend(&bluetooth, serverInstance, &cli, &networkSwitch);

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
