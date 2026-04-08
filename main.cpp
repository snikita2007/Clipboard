#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "WindowEffects.h"
#include "backend.h"

int main(int argc, char *argv[]) {
    // Установка стиля и базовых переменных
    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");
    QGuiApplication app(argc, argv); 

    QQmlApplicationEngine engine;
    
    // Создаем объекты логики
    WindowEffects windowEffects;
    Backend backend; // NetworkManager создастся внутри конструктора Backend

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
