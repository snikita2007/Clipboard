#include "backend.h"

void Backend::doWork(){
    qDebug() << "Работаем на платформе:" << QGuiApplication::platformName();
}

void Backend::clearClip(QObject* content){
    qDebug() << "Вызвана функция clearClip";
}