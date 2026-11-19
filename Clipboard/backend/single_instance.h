#ifndef SINGLE_INSTANCE_H
#define SINGLE_INSTANCE_H

#include <QByteArray>
#include <QObject>
#include <QString>

class QLocalServer;

// Keeps one running copy of the app per user. A later launch (app menu, a system
// keyboard shortcut) passes its command to the running copy and exits.
class SingleInstance : public QObject {
    Q_OBJECT

public:
    // socketPath is for tests; by default the socket is in the user's runtime directory.
    explicit SingleInstance(QObject *parent = nullptr, const QString &socketPath = QString());

    // True when another copy is running and has received the command.
    bool sendToRunning(const QByteArray &command);
    // Starts accepting commands from later launches.
    bool listen();

signals:
    void commandReceived(QByteArray command);

private:
    QString m_socketPath;
    QLocalServer *m_server = nullptr;
};

#endif
