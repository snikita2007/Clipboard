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
#include <QProcess>
#include <QCursor>

#include "NetworkManager.h"

class Backend : public QObject {
    Q_OBJECT

signals:
    void itemAdded(QVariant item);

public slots:
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

    Q_INVOKABLE void setLaunchAtStartup(bool enabled);
    Q_INVOKABLE void setHistoryLimit(int index);
    Q_INVOKABLE void setSaveClipboardHistory(bool enabled);
    Q_INVOKABLE void setDeviceSynk(bool enabled);
    Q_INVOKABLE void setAutoClear(int index);
    Q_INVOKABLE void setExcludeImages(bool enabled);
    Q_INVOKABLE void setClearOnExit(bool enabled);

private:
    void enforceAutoClear();

    QVariantList m_history;
    QString m_cacheDir;
    NetworkManager *m_network;

    bool m_launchAtStartup = false;
    int m_historyLimit = 0;
    bool m_saveClipboardHistory = true;
    bool m_deviceSynk = false;
    int m_autoClear = 0;
    bool m_excludeImages = false;
    bool m_clearOnExit = false;

    #ifdef Q_OS_MAC
    long m_lastChangeCount = -1; 
    int m_firstCount = 1;
    #endif
};

#endif