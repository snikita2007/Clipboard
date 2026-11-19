#pragma once

#include <QObject>
#include <QTimer>

// Remembers successful Clipboard connections and retries only while Bluetooth
// is ready. Keeping this policy separate lets it run without a settings page.
class BluetoothReconnect : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothReconnect(QObject *parent = nullptr);

    QString lastAddress() const { return m_lastAddress; }
    QString lastName() const { return m_lastName; }
    bool isActive() const { return m_enabled && !m_paused && !m_lastAddress.isEmpty(); }

    void setEnabled(bool enabled);
    void setReady(bool ready);
    void updateConnectionState(bool connected, bool connecting);
    void rememberDevice(const QString &address, const QString &name);
    // A manual disconnect or a manual choice of another device takes precedence.
    // A successful connection or re-entering Bluetooth mode resumes reconnecting.
    void pause();

signals:
    void connectRequested(const QString &address);
    void savedDeviceChanged(const QString &address, const QString &name);

private:
    void schedule(int delayMs);
    bool canConnect() const;

    QTimer m_retryTimer;
    QString m_lastAddress;
    QString m_lastName;
    bool m_enabled = false;
    bool m_ready = false;
    bool m_paused = false;
    bool m_connected = false;
    bool m_connecting = false;
    int m_retryDelayMs = 5000;
};
