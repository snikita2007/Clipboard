#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QtQml>
#include <QTimer>
#include <QDebug>
#include "server.h"
#include <QGuiApplication>
#include <QMimeData>
#include <QClipboard>
#include <QImage>
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QCryptographicHash>

class Backend : public QObject {
    Q_OBJECT // Позволяет классу использовать сигналы, слоты и систему свойств Qt
    QML_ELEMENT // Регистрирует класс как доступный тип в QML
    QML_SINGLETON // Создает экземпляр класса в единственном числе для всего QML приложения

signals:
    void itemAdded(QVariant item); // Сигнал о добавлении объекта в буфер

public slots:
    //Функция вызываемая при изменениии буфера, автоматически проверяет файлы на повторы и их тип, добавляет в историю
    void onClipboardChanged();

public:
    explicit Backend(UdpServer *server, QObject *parent = nullptr);
    ~Backend();
    Q_INVOKABLE void copyToClip(QVariant data); // Копирование в буфер из клиента
    void checkClipboardNative(); // Обновление буфера для мака
    Q_INVOKABLE void clearHistory();

private:
    QVariantList m_history;
    QString m_cacheDir;
    #ifdef Q_OS_MAC
    long m_lastChangeCount = -1; 
    #endif
};

#endif