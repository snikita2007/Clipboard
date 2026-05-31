#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QtQml>
#include <QTimer>
#include <QDebug>
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
#include "server.h"
#include "switch.h"
#include "bluetooth.h"
#include "NetworkManager.h"
#include "bluetooth_client.h"

#include <QCursor>

#include "NetworkManager.h"

class Backend : public QObject {
    Q_OBJECT // Позволяет классу использовать сигналы, слоты и систему свойств Qt

signals:
    void itemAdded(QVariant item); // Сигнал о добавлении объекта в буфер

public slots:
    //Функция вызываемая при изменениии буфера, автоматически проверяет файлы на повторы и их тип, добавляет в историю
    void onClipboardChanged();
    void handleRemoteData(QVariantMap data);

public:
    explicit Backend(BluetoothServer *bluetooth, UdpServer *server, BluetoothClient *cli, NetworkSwitch *m_networkSwitch , QObject *parent = nullptr);
    ~Backend();

    Q_INVOKABLE void copyToClip(QVariant data); // Копирование в буфер из клиента
    void checkClipboardNative(); // Обновление буфера для мака
    bool isDublicate(const QString &hash);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE QPoint getGlobalMousePos();
    QString generateHash(const QMimeData *mimeData);

private:
    QVariantList m_history;
    QString m_cacheDir;
    NetworkManager *m_network;
    #ifdef Q_OS_MAC
    long m_lastChangeCount = -1; 
    int m_firstCount = 1;
    #endif
};

#endif