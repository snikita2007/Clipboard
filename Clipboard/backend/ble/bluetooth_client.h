#ifndef BLUETOOTHCLIENT_H
#define BLUETOOTHCLIENT_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QVariantList>
#include <QTimer>
#include "bluetooth_transfer.h"

class BluetoothClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesDiscovered)
    Q_PROPERTY(bool discovering READ isDiscovering NOTIFY discoveringChanged)
    Q_PROPERTY(bool connecting READ isConnecting NOTIFY stateChanged)
    Q_PROPERTY(QString connectedAddress READ connectedAddress NOTIFY stateChanged)

public:
    explicit BluetoothClient(QObject *parent = nullptr);
    ~BluetoothClient();

    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void connectToDevice(const QString &addressStr);
    Q_INVOKABLE void disconnectFromDevice();

    void sendMessage(const QString &message);
    void sendFrame(const QByteArray &frame);
    void stopClient();
    void reconnectToDevice(const QString &address);
    void setSavedDevice(const QString &address, const QString &name);

    bool isConnected();
    bool isConnecting() const;
    bool isDiscovering() const;
    QString connectedAddress() const;
    QString peerName() const;
    QVariantList devices() const { return foundDevices; }

signals:
    void connectionRequested();
    void disconnectRequested();
    void messageReceived(const QString &sender, const QString &message);
    void dataReceived(const QVariantMap &item);
    void transferError(const QString &error);
    void transferFinished();
    void connected(const QString &name);
    void disconnected();
    void socketErrorOccurred(const QString &error);
    void devicesDiscovered(const QVariantList &devicesList);
    void discoveringChanged();
    void stateChanged();

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onDiscoveryFinished();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError error);
    void connectedSlot();
    void onSocketDisconnected();

private:
    void addDevice(const QString &name, const QString &address, bool paired);
    void addPairedDevices();
    void releaseSocket();
    void ensureDiscoveryAgent();
    void openConnection(const QString &address);

    QBluetoothSocket *socket = nullptr;
    BluetoothTransfer *m_transfer = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    QVariantList foundDevices;
    QString m_targetAddress;
    QString m_savedAddress;
    QString m_savedName;
    QTimer m_connectionTimer;
};


#endif
