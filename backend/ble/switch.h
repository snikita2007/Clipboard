#pragma once
#include <QObject>
#include "server.h"
#include "ble/bluetooth.h"
#include "ble/bluetooth_client.h"

class NetworkSwitch : public QObject {
    Q_OBJECT
public:
    enum TransportMode { Wifi, Bluetooth, None };

    NetworkSwitch(UdpServer *udp, BluetoothServer *btServer, BluetoothClient *btClient, QObject *parent = nullptr);
    TransportMode currentMode() const { return m_currentMode; }

    Q_INVOKABLE QString toggleMode();
    Q_INVOKABLE QString currentModeString();
    void setMode(TransportMode mode); 

signals:
    void modeChanged(QString newMode);

private:
    UdpServer *m_udp;
    BluetoothServer *m_btServer;
    BluetoothClient *m_btClient;
    TransportMode m_currentMode = TransportMode::None;
};
