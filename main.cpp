#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickWindow>
#include <QTimer>
#include "WindowEffects.h"
#include "backend.h"

int main(int argc, char *argv[]) {

    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    WindowEffects windowEffects;
    Backend backend;

    engine.rootContext()->setContextProperty("windowEffects", &windowEffects);
    engine.rootContext()->setContextProperty("myBackend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/Clipboard/main.qml")));

    QObject *root = engine.rootObjects().first();
    QQuickWindow *window = qobject_cast<QQuickWindow*>(root);

    if (window) {
    windowEffects.applyEffects(window);
    window->showMinimized();
    window->showNormal();
    window->show();
    }

    return app.exec();
}
