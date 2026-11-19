#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QClipboard>
#include <QImage>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QCryptographicHash>
#include <QProcess>
#include <QCursor>
#include <QBuffer>
#include <QSettings>

#include "ip.h"
#include "server.h"
#include "bluetooth.h"
#include "bluetooth_client.h"
#include "NetworkManager.h"
#include "switch.h"

class Backend : public QObject {
    Q_OBJECT

signals:
    void itemAdded(QVariant item);

public slots:
    void onClipboardChanged();
    void handleRemoteData(QVariantMap data);

public:
    explicit Backend(BluetoothServer *bluetooth, UdpServer *server, BluetoothClient *cli, NetworkSwitch *m_networkSwitch,NetworkScanner *m_networkscanner , QObject *parent = nullptr, const QString &cacheDir = QString());
    ~Backend();

    Q_INVOKABLE void copyToClip(QVariant data); // Копирование в буфер из клиента
    // Text copied on this device but read outside of QClipboard (Android background reader, Share menu)
    void processExternalText(const QString &text);
    void processExternalFiles(const QStringList &paths);
    // Android: how copied text gets sent while the app is in the background
    Q_INVOKABLE QString backgroundSendStatus();
    void checkClipboardNative(); // Обновление буфера для мака
    bool isDublicate(const QString &hash);
    Q_INVOKABLE void clearHistory();
    Q_INVOKABLE QPoint getGlobalMousePos();
    QString generateHash(const QMimeData *mimeData);

    Q_INVOKABLE void setLaunchAtStartup(bool enabled);
    // Creates or removes the login startup entry of the operating system
    static void applyLaunchAtStartup(bool enabled);
    Q_INVOKABLE void setHistoryLimit(int index);
    Q_INVOKABLE void setSaveClipboardHistory(bool enabled);
    Q_INVOKABLE void setDeviceSync(bool enabled);
    Q_INVOKABLE void setAutoClear(int index);
    Q_INVOKABLE void setExcludeImages(bool enabled);
    Q_INVOKABLE void setClearOnExit(bool enabled);

    bool main_flag = true;

private:
    void enforceAutoClear();
    void processMimeData(const QMimeData *mimeData, bool fromRemote);
    void broadcastItem(const QVariantMap &item);
    void sendPendingBluetooth();

    QVariantList m_history;
    QString m_cacheDir;

    BluetoothServer *m_bluetooth = nullptr;
    BluetoothClient *m_btClient = nullptr;
    NetworkSwitch *m_networkSwitch = nullptr;

    // Match the actual clipboard content, including delayed clipboard callbacks.
    QString m_lastRemoteHash;
    QByteArray m_pendingBluetoothFrame;

    bool m_launchAtStartup = false;
    int m_historyLimit = 50;
    bool m_saveClipboardHistory = true;
    bool m_deviceSynk = true;
    int m_autoClear = 0;
    bool m_excludeImages = false;
    bool m_clearOnExit = false;

    #ifdef Q_OS_MAC
    long m_lastChangeCount = -1; 
    int m_firstCount = 1;
    #endif
};

#endif

