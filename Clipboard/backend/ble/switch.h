#pragma once
#include <QObject>
#include <QBluetoothLocalDevice>
#include <QTimer>
#include "server.h"
#include "ble/bluetooth.h"
#include "ble/bluetooth_client.h"
#include "ble/bluetooth_reconnect.h"

// Chooses how clipboard items travel to other devices: Wi-Fi (local network) or Bluetooth.
class NetworkSwitch : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentMode READ currentModeString NOTIFY modeChanged)
    Q_PROPERTY(QString bluetoothStatus READ bluetoothStatus NOTIFY bluetoothStatusChanged)
    Q_PROPERTY(bool bluetoothConnected READ bluetoothConnected NOTIFY bluetoothStatusChanged)
    Q_PROPERTY(int transportIndex READ transportIndex NOTIFY modeChanged)
    Q_PROPERTY(bool bluetoothOnStartup READ bluetoothOnStartup WRITE setBluetoothOnStartup NOTIFY bluetoothOnStartupChanged)
    Q_PROPERTY(QString bluetoothTransferStatus READ bluetoothTransferStatus NOTIFY bluetoothTransferStatusChanged)

public:
    enum TransportMode { Wifi, Bluetooth, None };

    NetworkSwitch(UdpServer *udp, BluetoothServer *btServer, BluetoothClient *btClient, QObject *parent = nullptr);
    TransportMode currentMode() const { return m_currentMode; }
    int transportIndex() const { return m_currentMode == Bluetooth ? 1 : 0; }
    bool bluetoothOnStartup() const { return m_bluetoothOnStartup; }
    void setBluetoothOnStartup(bool enabled);
    // Called once after the application and its UI are ready for permission prompts.
    void start();

    Q_INVOKABLE QString toggleMode();
    Q_INVOKABLE QString currentModeString();
    // 0 = Wi-Fi, 1 = Bluetooth (same order as in the settings page)
    Q_INVOKABLE void setTransport(int index);
    // Starts the Bluetooth server if it is not running yet and searches for devices again
    Q_INVOKABLE void refreshBluetooth();
    void setMode(TransportMode mode);

    QString bluetoothStatus() const { return m_bluetoothStatus; }
    bool bluetoothConnected() const;
    QString bluetoothTransferStatus() const { return m_bluetoothTransferStatus; }
    void reportBluetoothTransfer(const QString &status);

signals:
    void modeChanged(QString newMode);
    void bluetoothStatusChanged();
    void bluetoothOnStartupChanged();
    void bluetoothTransferStatusChanged();

private:
    void startBluetooth();
    void startBluetoothWithPermission();
    void applyMode();
    void updateConnectionState();
    void updateBluetoothReadiness();
    void updateBluetoothStatus(const QString &error = QString());

    UdpServer *m_udp;
    BluetoothServer *m_btServer;
    BluetoothClient *m_btClient;
    TransportMode m_currentMode = TransportMode::None;
    QString m_bluetoothStatus;
    QString m_bluetoothTransferStatus;
    BluetoothReconnect m_reconnect;
    QBluetoothLocalDevice *m_localDevice = nullptr;
    QTimer m_startRetryTimer;
    QTimer m_powerOnTimer;
    bool m_started = false;
    bool m_bluetoothOnStartup = true;
    bool m_permissionPending = false;
    bool m_powerOnRequested = false;
    bool m_discoveryRequested = false;
};
