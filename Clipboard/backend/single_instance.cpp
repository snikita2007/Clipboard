#include "single_instance.h"
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>

SingleInstance::SingleInstance(QObject *parent, const QString &socketPath)
    : QObject(parent), m_socketPath(socketPath) {
    if (m_socketPath.isEmpty()) {
        // /run/user/<uid>: private to the user and cleared at logout
        m_socketPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + "/kngc-clipboard.sock";
    }
}

bool SingleInstance::sendToRunning(const QByteArray &command) {
    QLocalSocket socket;
    socket.connectToServer(m_socketPath);
    if (!socket.waitForConnected(500)) return false;

    socket.write(command + '\n');
    if (!socket.waitForBytesWritten(1000)) return false;
    socket.disconnectFromServer();
    return true;
}

bool SingleInstance::listen() {
    // A socket file can remain after a crash; no running copy answered on it.
    QLocalServer::removeServer(m_socketPath);

    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!m_server->listen(m_socketPath)) {
        qWarning() << "[SingleInstance] cannot listen on" << m_socketPath << ":" << m_server->errorString();
        return false;
    }

    connect(m_server, &QLocalServer::newConnection, this, [this]() {
        while (QLocalSocket *socket = m_server->nextPendingConnection()) {
            connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
                while (socket->canReadLine())
                    emit commandReceived(socket->readLine().trimmed());
            });
            connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        }
    });
    return true;
}
