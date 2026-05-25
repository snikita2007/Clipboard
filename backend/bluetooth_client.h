#ifndef BLUETOOTHCLIENT_H
#define BLUETOOTHCLIENT_H

#include <QObject>
#include <QBluetoothSocket>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QVariantList>

class BluetoothClient : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothClient(QObject *parent = nullptr);
    ~BluetoothClient();

    // Методы, доступные из QML и main.cpp
    Q_INVOKABLE void startDiscovery();
    Q_INVOKABLE void connectToDevice(const QString &addressStr);
    
    void sendMessage(const QString &message);
    void stopClient();

signals:
    void messageReceived(const QString &sender, const QString &message);
    void connected(const QString &name); // Сигнал с именем для UI
    void disconnected();
    void socketErrorOccurred(const QString &error);
    void devicesDiscovered(const QVariantList &devicesList);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &device);
    void onDiscoveryFinished();
    void readSocket();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError error);
    void connectedSlot(); // Внутренний слот для обработки успешного соединения

private:
    QBluetoothSocket *socket = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    QVariantList foundDevices; 
};


#endif 

