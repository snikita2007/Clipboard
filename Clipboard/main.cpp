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
#else
#include "android_clipboard.h"
#endif

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
#include <QSettings>
#include <cstdio>
#include "single_instance.h"

// "--autostart=on|off" lets the installer set login startup without opening the app.
// Returns the exit code, or -1 when the option is absent.
static int runAutostartOption(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        const QByteArray option(argv[i]);
        if (!option.startsWith("--autostart=")) continue;

        const QByteArray state = option.mid(int(qstrlen("--autostart=")));
        if (state != "on" && state != "off") {
            fprintf(stderr, "Usage: %s --autostart=on|off\n", argv[0]);
            return 2;
        }
        QCoreApplication app(argc, argv);
        // The same value as the "Launch at startup" switch in Settings
        QSettings settings;
        settings.setValue("ApplicationSettings/launchAtStartup", state == "on");
        settings.sync();
        Backend::applyLaunchAtStartup(state == "on");
        return settings.status() == QSettings::NoError ? 0 : 1;
    }
    return -1;
}
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
        qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
        // The global hotkey, opening at the cursor and clipboard tracking in the background
        // need X11. On a Wayland session the app runs through XWayland.
        if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariableIsSet("DISPLAY"))
            qputenv("QT_QPA_PLATFORM", "xcb");
    #endif

    QCoreApplication::setOrganizationName("KNGC");
    QCoreApplication::setOrganizationDomain("kngc.com");
    QCoreApplication::setApplicationName("ClipboardApp");

    #if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    const int autostartExitCode = runAutostartOption(argc, argv);
    if (autostartExitCode >= 0) return autostartExitCode;
    #endif

    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/ui/assets/app.png"));

    #if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    // A second launch (app menu, a system keyboard shortcut) controls the running copy.
    SingleInstance singleInstance;
    const bool toggleRequested = app.arguments().contains(QStringLiteral("--toggle"));
    if (singleInstance.sendToRunning(toggleRequested ? "toggle" : "show")) return 0;
    singleInstance.listen();
    #endif

    UdpServer *serverInstance = &UdpServer::instance();

    NetworkScanner scanner;
    BluetoothServer bluetooth;
    BluetoothClient cli;

    NetworkSwitch networkSwitch(serverInstance, &bluetooth, &cli);

    scanner.startScanning(1234);
    
    QObject::connect(&scanner,&NetworkScanner::devicesUpdated,serverInstance,&UdpServer::updateIps,Qt::QueuedConnection);

    QQmlApplicationEngine engine;
    WindowEffects windowEffects;
    
    Backend backend(&bluetooth, serverInstance, &cli, &networkSwitch, &scanner);

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

    // Start independently of the settings page, once the event loop can handle
    // Bluetooth permissions and asynchronous adapter power-on.
    QTimer::singleShot(0, &networkSwitch, &NetworkSwitch::start);

    #ifdef Q_OS_ANDROID
    // Background service: keeps sync running and reads the clipboard in the background
    AndroidClipboard::start(&backend);
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() { AndroidClipboard::stop(); });
    #endif

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

    #if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QObject::connect(&singleInstance, &SingleInstance::commandReceived, &app, [&engine](const QByteArray &command) {
        if (engine.rootObjects().isEmpty()) return;
        QObject *root = engine.rootObjects().first();
        const bool hide = command == "toggle" && root->property("visible").toBool();
        QMetaObject::invokeMethod(root, hide ? "softClose" : "softOpen");
    });
    #endif

    // Shortcuts and the installer open the UI; login startup still stays in the tray.
    if (app.arguments().contains(QStringLiteral("--show")) || app.arguments().contains(QStringLiteral("--toggle"))) {
        QTimer::singleShot(350, &engine, [&engine]() {
            if (!engine.rootObjects().isEmpty())
                QMetaObject::invokeMethod(engine.rootObjects().first(), "softOpen");
        });
    }

    return app.exec();
}
