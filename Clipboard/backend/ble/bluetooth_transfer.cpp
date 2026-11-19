#include "bluetooth_transfer.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

namespace {
QString message(const char *text) { return BluetoothTransfer::tr(text); }

bool fail(QString *error, const QString &text)
{
    if (error) *error = text;
    return false;
}

bool decodeBytes(const QByteArray &encoded, QByteArray *bytes)
{
    const auto result = QByteArray::fromBase64Encoding(encoded, QByteArray::AbortOnBase64DecodingErrors);
    if (!result) return false;
    *bytes = *result;
    return true;
}
}

QByteArray BluetoothProtocol::encodeText(const QString &text)
{
    return "T:" + text.toUtf8().toBase64() + '\n';
}

QByteArray BluetoothProtocol::encodeItem(const QVariantMap &item, QString *error)
{
    if (error) error->clear();
    const QString type = item.value("type").toString();
    if (type == "text") {
        const QString text = item.value("data").toString();
        if (text.toUtf8().size() <= MaxPayloadBytes) return encodeText(text);
        fail(error, message("Bluetooth clipboard content exceeds 5 MB."));
        return {};
    }

    if (type != "image" && type != "file" && type != "files") {
        fail(error, message("Unsupported Bluetooth clipboard content."));
        return {};
    }
    const QStringList paths = type == "files" ? item.value("paths").toStringList()
                                               : QStringList{item.value("data").toString()};
    if (paths.isEmpty() || paths.size() > MaxFiles) {
        fail(error, message("Select between 1 and 32 files for Bluetooth sync."));
        return {};
    }

    QJsonArray attachments;
    qint64 total = 0;
    for (const QString &path : paths) {
        const QFileInfo info(path);
        QFile file(path);
        if (!info.isFile() || !file.open(QIODevice::ReadOnly)) {
            fail(error, message("Cannot read %1.").arg(info.fileName()));
            return {};
        }
        const qint64 remaining = MaxPayloadBytes - total;
        if (file.size() > remaining) {
            fail(error, message("Bluetooth sends photos and files up to 5 MB in total. Use Wi-Fi for larger items."));
            return {};
        }
        // Also bound the read if the file changes after checking its size.
        const QByteArray bytes = file.read(remaining + 1);
        if (bytes.size() > remaining || file.error() != QFileDevice::NoError || !file.atEnd()) {
            fail(error, message("Cannot read %1 within the 5 MB Bluetooth limit.").arg(info.fileName()));
            return {};
        }
        total += bytes.size();
        const QString name = info.fileName();
        if (name.isEmpty() || name.toUtf8().size() > 1024) {
            fail(error, message("Invalid attachment name."));
            return {};
        }
        attachments.append(QJsonObject{{"type", type == "image" ? "image" : "file"},
                                       {"name", name}, {"data", QString::fromLatin1(bytes.toBase64())}});
    }
    return "A:" + QJsonDocument(QJsonObject{{"v", 1}, {"items", attachments}}).toJson(QJsonDocument::Compact) + '\n';
}

bool BluetoothProtocol::decodeLine(const QByteArray &line, QVariantMap *item, QString *error)
{
    if (error) error->clear();
    item->clear();
    if (line.isEmpty()) return false;
    if (line.size() > MaxFrameBytes)
        return fail(error, message("Bluetooth clipboard content exceeds 5 MB."));

    if (!line.startsWith("A:")) {
        QByteArray bytes = line;
        if (line.startsWith("T:") && !decodeBytes(line.mid(2), &bytes))
            return fail(error, message("Invalid Bluetooth text message."));
        if (bytes.size() > MaxPayloadBytes)
            return fail(error, message("Bluetooth clipboard content exceeds 5 MB."));
        *item = {{"type", "text"}, {"data", QString::fromUtf8(bytes)}};
        return true;
    }

    const QJsonDocument document = QJsonDocument::fromJson(line.mid(2));
    const QJsonObject root = document.object();
    const QJsonArray attachments = root.value("items").toArray();
    if (root.value("v").toDouble() != 1 || attachments.isEmpty() || attachments.size() > MaxFiles)
        return fail(error, message("Invalid Bluetooth attachment message."));

    QVariantList files;
    qint64 total = 0;
    for (const QJsonValue &value : attachments) {
        const QJsonObject attachment = value.toObject();
        const QString type = attachment.value("type").toString();
        const QString name = attachment.value("name").toString();
        const QJsonValue encoded = attachment.value("data");
        QByteArray bytes;
        if ((type != "image" && type != "file") || name.isEmpty() || name.toUtf8().size() > 1024
                || !encoded.isString() || !decodeBytes(encoded.toString().toLatin1(), &bytes))
            return fail(error, message("Invalid Bluetooth attachment message."));
        total += bytes.size();
        if (total > MaxPayloadBytes)
            return fail(error, message("Bluetooth attachments exceed 5 MB in total."));
        files.append(QVariantMap{{"type", type}, {"file_name", name},
                                 {type == "image" ? "img" : "file_bytes", bytes}});
    }
    *item = files.size() == 1 ? files.first().toMap()
                              : QVariantMap{{"type", "files"}, {"files", files}};
    return true;
}

BluetoothTransfer::BluetoothTransfer(QIODevice *device, QObject *parent)
    : QObject(parent), m_device(device)
{
    connect(device, &QIODevice::readyRead, this, &BluetoothTransfer::readAvailable);
    connect(device, &QIODevice::bytesWritten, this, &BluetoothTransfer::writeAvailable, Qt::QueuedConnection);
    // Data can already be buffered when an incoming connection is accepted.
    QMetaObject::invokeMethod(this, &BluetoothTransfer::readAvailable, Qt::QueuedConnection);
}

bool BluetoothTransfer::sendFrame(const QByteArray &frame)
{
    if (frame.isEmpty() || !m_device || !m_device->isWritable()) return false;
    if (frame.size() > BluetoothProtocol::MaxFrameBytes + 1
            || m_output.size() >= 32
            || m_queuedBytes + m_device->bytesToWrite() + frame.size() > 2 * (BluetoothProtocol::MaxFrameBytes + 1)) {
        emit transferError(tr("Bluetooth is busy. Wait for the current transfer, then copy again."));
        return false;
    }
    m_output.enqueue(frame);
    m_queuedBytes += frame.size();
    m_sending = true;
    writeAvailable();
    return m_device && m_device->isWritable();
}

void BluetoothTransfer::stop()
{
    if (m_device) m_device->disconnect(this);
    m_device = nullptr;
    m_input.clear();
    m_output.clear();
    m_offset = m_queuedBytes = 0;
    m_sending = false;
}

void BluetoothTransfer::writeAvailable()
{
    if (!m_device) return;
    constexpr qint64 chunkSize = 16 * 1024;
    constexpr qint64 highWater = 64 * 1024;
    while (!m_output.isEmpty() && m_device->isWritable() && m_device->bytesToWrite() < highWater) {
        const QByteArray &frame = m_output.head();
        const qint64 count = qMin(chunkSize, frame.size() - m_offset);
        const qint64 written = m_device->write(frame.constData() + m_offset, count);
        if (!m_device) return; // A socket error may synchronously release the connection.
        if (written < 0) {
            const QString error = tr("Bluetooth transfer failed: %1").arg(m_device->errorString());
            QIODevice *device = m_device;
            stop();
            emit transferError(error);
            // A partially written frame cannot be followed by another frame.
            device->close();
            return;
        }
        if (written == 0) {
            QTimer::singleShot(10, this, &BluetoothTransfer::writeAvailable);
            return;
        }
        m_offset += written;
        m_queuedBytes -= written;
        if (m_offset == frame.size()) {
            m_output.dequeue();
            m_offset = 0;
        }
    }
    if (m_sending && m_output.isEmpty() && m_device->bytesToWrite() == 0) {
        m_sending = false;
        emit finished();
    }
}

void BluetoothTransfer::readAvailable()
{
    while (m_device && m_device->bytesAvailable() > 0) {
        const QByteArray chunk = m_device->read(16 * 1024);
        if (!m_device || chunk.isEmpty()) break;
        qsizetype offset = 0;
        while (offset < chunk.size()) {
            const qsizetype newline = chunk.indexOf('\n', offset);
            const qsizetype end = newline < 0 ? chunk.size() : newline;
            if (!m_discardLine) {
                if (m_input.size() + end - offset > BluetoothProtocol::MaxFrameBytes) {
                    m_input.clear();
                    m_discardLine = true;
                    emit transferError(tr("Bluetooth clipboard content exceeds 5 MB."));
                } else {
                    m_input.append(chunk.constData() + offset, end - offset);
                }
            }
            if (newline >= 0) {
                if (!m_discardLine) {
                    if (m_input.endsWith('\r')) m_input.chop(1);
                    QVariantMap item;
                    QString error;
                    const bool decoded = BluetoothProtocol::decodeLine(m_input, &item, &error);
                    m_input.clear();
                    if (decoded) emit itemReceived(item);
                    else if (!error.isEmpty()) emit transferError(error);
                    if (!m_device) return;
                }
                m_input.clear();
                m_discardLine = false;
            }
            offset = end + 1;
        }
    }
}
