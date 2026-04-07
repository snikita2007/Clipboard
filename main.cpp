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
#include "examplethreads.h"
#include <fstream>

using namespace std;

int main(int argc, char *argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv);
    
    ExampleThreads threadA("test"); 

    QStringList IPS;
    
    UdpServer server;

    server.startServer(1234);    

    QObject::connect(&UdpServer::instance(), &UdpServer::dataReceived, [&IPS](QStringList ips) {
        qDebug() << "Получено в main:" << ips;
        IPS.append(ips);
    });

    QObject::connect(&threadA, &QThread::finished, [&IPS, &server]() {
        qDebug() << "поток завершен";

        qDebug() << IPS;
        server.Recive(IPS);

        ofstream out("/home/danil/Documents/Clipboard/ips.txt");
    
        if (out.is_open()) {
            for (QString &ip : IPS)
            out << ip.toStdString() << endl;
        }
    
        out.close();
    });
    
    threadA.start();   

    QQmlApplicationEngine engine;
    WindowEffects windowEffects;
    Backend backend(&server);

    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/Clipboard/ui/main.qml")));

    QObject *root = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

    // Наложение эффектов окна 
    if (window) {
        windowEffects.applyEffects(window);
        window->show();
    }

    return app.exec();
}


// int main(int argc, char *argv[]) {
//     qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
//     QGuiApplication app(argc, argv);

//     QQmlApplicationEngine engine;
//     WindowEffects windowEffects;
//     Backend backend(&server);

//     engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
//     engine.rootContext()->setContextProperty("backend", &backend);

//     engine.load(QUrl(QStringLiteral("qrc:/Clipboard/ui/main.qml")));

//     QObject *root = engine.rootObjects().first();
//     QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

//     if (window) {
//         windowEffects.applyEffects(window);
//         window->show();
//     }

//     return app.exec();
// }

