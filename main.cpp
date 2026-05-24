#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "QIcon"

#ifndef Q_OS_ANDROID 
#include <QHotkey> 
#endif

#include "WindowEffects.h"
#include "backend.h"

int main(int argc, char *argv[]) {
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QApplication app(argc, argv); 

    app.setWindowIcon(QIcon(":/ui/assets/app.png"));

    QQmlApplicationEngine engine;
    
    WindowEffects windowEffects;
    Backend backend;
   
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
            window->show();
        }
    }

    return app.exec();
}
