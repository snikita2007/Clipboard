#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "WindowEffects.h"
#include "backend.h"
<<<<<<< HEAD
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

#include "WindowEffects.h"
#include "backend.h"
=======
>>>>>>> vibe-branch

int main(int argc, char *argv[]) {
    // Установка стиля и базовых переменных
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv); 
<<<<<<< HEAD

    UdpServer server;
    NetworkScanner scanner;
    
    scanner.startScanning(1234);
    
    server.startServer(1234);    

    BluetoothServer bluetooth;
    BluetoothClient cli;
    
    QBluetoothAddress adapterAddress;
    QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (!devices.isEmpty()) {
        adapterAddress = devices.first().address();
    }
    
    bluetooth.startServer(adapterAddress);
                 
    // QBluetoothAddress address("F8:5C:7D:2F:20:95");
    // QBluetoothUuid uuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
=======
>>>>>>> vibe-branch

    // cli.startClient(address, uuid);

    QObject::connect(&scanner, &NetworkScanner::devicesUpdated, &server, &UdpServer::updateIps);
    
    QQmlApplicationEngine engine;
    
    // Создаем объекты логики
    WindowEffects windowEffects;
<<<<<<< HEAD
    Backend backend(&bluetooth,&server);
 
=======
    Backend backend; // NetworkManager создастся внутри конструктора Backend

>>>>>>> vibe-branch
    // Регистрируем объекты в QML контексте
    // ВАЖНО: используйте маленькую букву "backend", если в QML обращаетесь к ней так
    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
    engine.rootContext()->setContextProperty("backend", &backend);

    // Загружаем основной файл интерфейса
    // Убедитесь, что путь верный (у вас в коде было два разных пути)
    const QUrl url(QStringLiteral("qrc:/Clipboard/ui/main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    // Работа с окном (эффекты)
    if (!engine.rootObjects().isEmpty()) {
        QObject *root = engine.rootObjects().first();
        QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

        if (window) {
            windowEffects.applyEffects(window);
            window->show();
        }
    }

    return app.exec();
}
