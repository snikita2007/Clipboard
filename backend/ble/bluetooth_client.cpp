#include "bluetooth_client.h"
#include <QMetaEnum>
#include <QDebug>

using namespace Qt::StringLiterals;

static const QBluetoothUuid serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");

BluetoothClient::BluetoothClient(QObject *parent) 
    : QObject(parent) 
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothClient::onDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothClient::onDiscoveryFinished);
}

BluetoothClient::~BluetoothClient() {
    stopClient();
}

void BluetoothClient::startDiscovery() {
    foundDevices.clear();
    discoveryAgent->start();
    qDebug() << "SCANING...";
}

void BluetoothClient::onDeviceDiscovered(const QBluetoothDeviceInfo &device) {
    QVariantMap deviceMap;
    deviceMap["name"] = device.name().isEmpty() ? "UNKNOWN - DEVICE"_L1 : device.name();
    deviceMap["address"] = device.address().toString();

    for (const auto &item : foundDevices) {
        if (item.toMap()["address"].toString() == device.address().toString())
            return;
    }

    foundDevices.append(deviceMap);
    emit devicesDiscovered(foundDevices);
}

void BluetoothClient::onDiscoveryFinished() {
    qDebug() << "FINISHED:" << foundDevices.count() << foundDevices;
}

bool BluetoothClient::isConnected() {
    if (socket && socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        return true;
    }
    return false;
}

void BluetoothClient::connectToDevice(const QString &addressStr) {
    if (socket) return;

    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    
    connect(socket, &QBluetoothSocket::readyRead, this, &BluetoothClient::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, &BluetoothClient::connectedSlot);
    connect(socket, &QBluetoothSocket::disconnected, this, &BluetoothClient::disconnected);
    connect(socket, &QBluetoothSocket::errorOccurred, this, &BluetoothClient::onSocketErrorOccurred);

    QBluetoothAddress address(addressStr);
    socket->connectToService(address, serviceUuid); 
    qDebug() << "CONNECTED" << addressStr << "...";
}

void BluetoothClient::stopClient() {
    if (socket) {
        socket->disconnectFromService();
        delete socket;
        socket = nullptr;
    }
}

void BluetoothClient::readSocket() {
    if (!socket) return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(socket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}

void BluetoothClient::sendMessage(const QString &message) {
    if (socket && socket->isOpen()) {
        QByteArray text = message.toUtf8() + '\n';
        socket->write(text);
    }
}

void BluetoothClient::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    if (error == QBluetoothSocket::SocketError::NoSocketError) return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    QString errorString = (socket ? socket->peerName() : "Unknown"_L1) + ' '_L1
            + metaEnum.valueToKey(static_cast<int>(error)) + " occurred"_L1;

    emit socketErrorOccurred(errorString);
}

void BluetoothClient::connectedSlot() {
    if (socket) {
        emit connected(socket->peerName());
    }
}
