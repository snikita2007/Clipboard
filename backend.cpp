#include "backend.h"

void Backend::doWork(){
    qDebug() << "Работаем на платформе:" << QGuiApplication::platformName();
}