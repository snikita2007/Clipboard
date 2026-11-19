#ifndef BLUETOOTHSERVER_H
#define BLUETOOTHSERVER_H

#include <QObject>
#include <QBluetoothServer>
#include <QBluetoothSocket>
#include <QBluetoothServiceInfo>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include "bluetooth_transfer.h"

class BluetoothServer : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothServer(QObject *parent = nullptr);
    ~BluetoothServer();

    bool startServer(const QBluetoothAddress &localAdapter = QBluetoothAddress());
    void stopServer();

    bool isRunning() const { return rfcommServer != nullptr; }
    bool hasActiveConnections();
    QStringList connectedNames() const;

public slots:
    void sendMessage(const QString &message);
    void sendFrame(const QByteArray &frame);

signals:
    void messageReceived(const QString &sender, const QString &message);
    void dataReceived(const QVariantMap &item);
    void transferError(const QString &error);
    void transferFinished();
    void clientConnected(const QString &name);
    void peerConnected(const QString &address, const QString &name);
    void clientDisconnected(const QString &name);
    void errorOccurred(const QString &error);

private slots:
    void clientConnected();
    void clientDisconnected();

private:
    QBluetoothServer *rfcommServer = nullptr;
    QBluetoothServiceInfo serviceInfo;
    QList<QBluetoothSocket *> clientSockets;
    QMap<QBluetoothSocket *, QString> clientNames;
    QMap<QBluetoothSocket *, BluetoothTransfer *> transfers;
};

#endif
