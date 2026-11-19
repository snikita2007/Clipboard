#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QDataStream>
#include <QVariantMap>
#include <QTimer>
#include <QNetworkInterface>


class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    // Метод для отправки данных всем найденным устройствам
    void broadcastClipboard(const QVariantMap &data);

signals:
    void dataReceived(QVariantMap data); // Сработает, когда придут данные от соседа
    void deviceFound(QString ip);        // Сработает, когда в сети появится новое устройство

private slots:
    void sendDiscoveryPacket();          // Рассылка "Я тут" (UDP)
    void processUdpPackets();            // Обработка ответов (UDP)
    void onNewTcpConnection();           // Прием входящего соединения (TCP)
    void readIncomingData();             // Чтение данных (TCP)

private:
    // Настройки сети
    const quint16 UDP_PORT = 45454;
    const quint16 TCP_PORT = 45455;
    const QHostAddress GROUP_ADDR = QHostAddress::Broadcast;

    QUdpSocket *m_udpSocket;
    QTcpServer *m_tcpServer;
    QTimer *m_discoveryTimer;
    
    // Список IP адресов активных соседей
    QSet<QHostAddress> m_knownDevices;
};

#endif // NETWORKMANAGER_H
