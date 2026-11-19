#pragma once

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QQueue>
#include <QVariantMap>

namespace BluetoothProtocol {
// Limit the entire clipboard selection, including all selected files.
constexpr qint64 MaxPayloadBytes = 5 * 1024 * 1024;
constexpr int MaxFiles = 32;
constexpr qint64 MaxFrameBytes = ((MaxPayloadBytes + 2) / 3) * 4 + 64 * 1024;

// T:base64(UTF-8) remains compatible with earlier text-only versions.
// A:JSON carries a version and an array of named, base64-encoded attachments.
// Both frame types end in '\n'; JSON escapes newlines in file names.
QByteArray encodeText(const QString &text);
QByteArray encodeItem(const QVariantMap &item, QString *error);
bool decodeLine(const QByteArray &line, QVariantMap *item, QString *error);
}

// One instance per connection. Limits buffering, handles fragmented reads and
// partial writes, and never blocks the GUI thread waiting for Bluetooth I/O.
class BluetoothTransfer : public QObject {
    Q_OBJECT
public:
    explicit BluetoothTransfer(QIODevice *device, QObject *parent = nullptr);
    bool sendFrame(const QByteArray &frame);
    void stop();

signals:
    void itemReceived(const QVariantMap &item);
    void transferError(const QString &error);
    void finished();

private:
    void readAvailable();
    void writeAvailable();

    QIODevice *m_device;
    QByteArray m_input;
    bool m_discardLine = false;
    QQueue<QByteArray> m_output;
    qint64 m_offset = 0;
    qint64 m_queuedBytes = 0;
    bool m_sending = false;
};
