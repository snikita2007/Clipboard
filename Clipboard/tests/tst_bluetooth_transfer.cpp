#include <QtTest>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "backend.h"
#include "bluetooth_transfer.h"
#include "clipboard_files.h"

// A sequential device that deliberately accepts short writes and only drains
// when instructed, so tests exercise fragmentation and Bluetooth backpressure.
class TestSocket : public QIODevice {
public:
    TestSocket() { open(QIODevice::ReadWrite | QIODevice::Unbuffered); }
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return input.size() + QIODevice::bytesAvailable(); }
    qint64 bytesToWrite() const override { return pending.size(); }
    void feed(const QByteArray &bytes) { input += bytes; emit readyRead(); }
    QByteArray drain(qint64 count) {
        const QByteArray bytes = pending.left(count);
        pending.remove(0, bytes.size());
        if (!bytes.isEmpty()) emit bytesWritten(bytes.size());
        return bytes;
    }
    QByteArray input;
    QByteArray pending;
    qint64 writeLimit = 127;
    bool failWrites = false;
protected:
    qint64 readData(char *data, qint64 size) override {
        const qint64 count = qMin(size, qint64(input.size()));
        memcpy(data, input.constData(), size_t(count));
        input.remove(0, count);
        return count;
    }
    qint64 writeData(const char *data, qint64 size) override {
        if (failWrites) { setErrorString("connection lost"); return -1; }
        const qint64 count = qMin(size, writeLimit);
        pending.append(data, count);
        return count;
    }
};

class BluetoothTransferTest : public QObject {
    Q_OBJECT
    QTemporaryDir m_directory;

    QString writeFile(const QString &name, const QByteArray &bytes) {
        const QString path = m_directory.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) return {};
        return path;
    }
    static QByteArray imageBytes() {
        QImage image(16, 12, QImage::Format_ARGB32);
        image.fill(QColor(23, 120, 240, 128));
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return bytes;
    }
    static QByteArray attachment(const QString &type, const QString &name, const QByteArray &bytes) {
        return "A:" + QJsonDocument(QJsonObject{{"v", 1}, {"items", QJsonArray{
            QJsonObject{{"type", type}, {"name", name}, {"data", QString::fromLatin1(bytes.toBase64())}}
        }}}).toJson(QJsonDocument::Compact) + '\n';
    }

private slots:
    void initTestCase() {
        QVERIFY(m_directory.isValid());
        QCoreApplication::setOrganizationName("ClipboardTests");
        QCoreApplication::setApplicationName("BluetoothTransfer");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_directory.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, m_directory.path());
    }
    void init() { QSettings().clear(); QGuiApplication::clipboard()->clear(); }

    void fragmentedTextAndLegacy() {
        TestSocket socket;
        BluetoothTransfer transfer(&socket);
        QSignalSpy received(&transfer, &BluetoothTransfer::itemReceived);
        const QString text = QString::fromUtf8("  Привет\nBluetooth 👋\t ");
        const QByteArray frame = BluetoothProtocol::encodeText(text);
        for (char byte : frame) socket.feed(QByteArray(1, byte));
        QCOMPARE(received.size(), 1);
        QCOMPARE(received[0][0].toMap().value("data").toString(), text);
        socket.feed(" legacy text \r\nT:dHdv\nT:dGhyZWU=\n");
        QCOMPARE(received.size(), 4);
        QCOMPARE(received[1][0].toMap().value("data").toString(), QString(" legacy text "));
        QCOMPARE(received[3][0].toMap().value("data").toString(), QString("three"));
    }

    void attachmentsRoundTrip_data() {
        QTest::addColumn<QString>("type");
        QTest::addColumn<QString>("name");
        QTest::addColumn<QByteArray>("bytes");
        QTest::newRow("photo") << QString("image") << QString("photo.png") << imageBytes();
        QTest::newRow("binary") << QString("file") << QString::fromUtf8("файл.bin") << QByteArray("\0\xff\n\rabc", 7);
        QTest::newRow("empty-file") << QString("file") << QString("empty.txt") << QByteArray();
        QTest::newRow("limit") << QString("file") << QString("limit.bin") << QByteArray(BluetoothProtocol::MaxPayloadBytes, 'x');
    }
    void attachmentsRoundTrip() {
        QFETCH(QString, type);
        QFETCH(QString, name);
        QFETCH(QByteArray, bytes);
        const QString path = writeFile(name, bytes);
        QVERIFY(!path.isEmpty());
        QString error;
        const QByteArray frame = BluetoothProtocol::encodeItem({{"type", type}, {"data", path}}, &error);
        QVERIFY2(!frame.isEmpty(), qPrintable(error));
        TestSocket sender;
        TestSocket receiver;
        BluetoothTransfer outgoing(&sender);
        BluetoothTransfer incoming(&receiver);
        QSignalSpy received(&incoming, &BluetoothTransfer::itemReceived);
        QSignalSpy finished(&outgoing, &BluetoothTransfer::finished);
        QVERIFY(outgoing.sendFrame(frame));
        int attempts = 0;
        while (finished.isEmpty() && ++attempts < 20000) {
            QVERIFY(sender.bytesToWrite() <= 80 * 1024);
            receiver.feed(sender.drain(1777));
            QCoreApplication::processEvents();
        }
        QCOMPARE(finished.size(), 1);
        QCOMPARE(received.size(), 1);
        const QVariantMap item = received[0][0].toMap();
        QCOMPARE(item.value("type").toString(), type);
        QCOMPARE(item.value("file_name").toString(), name);
        QCOMPARE(item.value(type == "image" ? "img" : "file_bytes").toByteArray(), bytes);
    }

    void multipleFilesStayTogether() {
        const QString first = writeFile("one.txt", "one");
        const QString second = writeFile("two.txt", "two");
        QString error;
        const QByteArray frame = BluetoothProtocol::encodeItem({{"type", "files"}, {"paths", QStringList{first, second}}}, &error);
        QVariantMap decoded;
        QVERIFY2(BluetoothProtocol::decodeLine(frame.chopped(1), &decoded, &error), qPrintable(error));
        QCOMPARE(decoded.value("type").toString(), QString("files"));
        const QVariantList files = decoded.value("files").toList();
        QCOMPARE(files.size(), 2);
        QCOMPARE(files[0].toMap().value("file_bytes").toByteArray(), QByteArray("one"));
        QCOMPARE(files[1].toMap().value("file_name").toString(), QString("two.txt"));
    }

    void rejectsOversizedOrUnreadableSelection() {
        QString error;
        const QString large = writeFile("large.bin", QByteArray(BluetoothProtocol::MaxPayloadBytes + 1, 'x'));
        QVERIFY(BluetoothProtocol::encodeItem({{"type", "file"}, {"data", large}}, &error).isEmpty());
        QVERIFY(error.contains("5 MB"));
        const QString half = writeFile("half.bin", QByteArray(BluetoothProtocol::MaxPayloadBytes / 2 + 1, 'x'));
        QVERIFY(BluetoothProtocol::encodeItem({{"type", "files"}, {"paths", QStringList{half, half}}}, &error).isEmpty());
        QVERIFY(BluetoothProtocol::encodeItem({{"type", "file"}, {"data", m_directory.path()}}, &error).isEmpty());
        QVERIFY(BluetoothProtocol::encodeItem({{"type", "file"}, {"data", m_directory.filePath("missing")}}, &error).isEmpty());
        QVERIFY(BluetoothProtocol::encodeItem({{"type", "files"}, {"paths", QStringList(33, half)}}, &error).isEmpty());
    }

    void malformedFramesAndRecovery() {
        TestSocket socket;
        BluetoothTransfer transfer(&socket);
        QSignalSpy errors(&transfer, &BluetoothTransfer::transferError);
        QSignalSpy received(&transfer, &BluetoothTransfer::itemReceived);
        socket.feed("T:%%%\nA:bad-json\nA:{\"v\":2,\"items\":[{}]}\n");
        socket.feed(attachment("unknown", "file", "content"));
        socket.feed("A:{\"v\":1,\"items\":[{\"type\":\"file\",\"name\":\"file\",\"data\":\"%%%\"}]}\n");
        socket.feed(attachment("file", "large", QByteArray(BluetoothProtocol::MaxPayloadBytes + 1, 'x')));
        QCOMPARE(received.size(), 0);
        QCOMPARE(errors.size(), 6);
        socket.feed(QByteArray(BluetoothProtocol::MaxFrameBytes + 10, 'x'));
        QCOMPARE(errors.size(), 7);
        socket.feed("discard this too\nT:b2s=\n");
        QCOMPARE(received.size(), 1);
        QCOMPARE(received[0][0].toMap().value("data").toString(), QString("ok"));
    }

    void interruptedReadDoesNotApplyPartialFile() {
        const QByteArray frame = attachment("file", "file.bin", "binary");
        TestSocket socket;
        {
            BluetoothTransfer transfer(&socket);
            QSignalSpy received(&transfer, &BluetoothTransfer::itemReceived);
            socket.feed(frame.left(frame.size() - 2));
            QCOMPARE(received.size(), 0);
        }
        BluetoothTransfer reconnected(&socket);
        QSignalSpy received(&reconnected, &BluetoothTransfer::itemReceived);
        socket.feed(frame);
        QCOMPARE(received.size(), 1);
    }

    void boundsQueueAndReportsWriteFailure() {
        TestSocket socket;
        BluetoothTransfer transfer(&socket);
        QSignalSpy errors(&transfer, &BluetoothTransfer::transferError);
        const QByteArray frame = attachment("file", "large.bin", QByteArray(BluetoothProtocol::MaxPayloadBytes, 'x'));
        QVERIFY(transfer.sendFrame(frame));
        QVERIFY(transfer.sendFrame(frame));
        QVERIFY(!transfer.sendFrame(frame));
        QCOMPARE(errors.size(), 1);
        socket.failWrites = true;
        socket.drain(16384);
        QCoreApplication::processEvents();
        QCOMPARE(errors.size(), 2);
        QVERIFY(!socket.isOpen());
    }

    void closingConnectionDiscardsPendingData() {
        TestSocket socket;
        BluetoothTransfer transfer(&socket);
        QSignalSpy received(&transfer, &BluetoothTransfer::itemReceived);
        QSignalSpy finished(&transfer, &BluetoothTransfer::finished);
        QVERIFY(transfer.sendFrame(attachment("file", "file.bin", QByteArray(100000, 'x'))));
        socket.feed("T:cGFydGlhbA==");
        transfer.stop();
        socket.drain(socket.bytesToWrite());
        socket.feed("\nT:bmV3\n");
        QCoreApplication::processEvents();
        QCOMPARE(socket.bytesToWrite(), 0);
        QCOMPARE(received.size(), 0);
        QCOMPARE(finished.size(), 0);
        QVERIFY(!transfer.sendFrame(BluetoothProtocol::encodeText("stale")));
    }

    void preservesOrderOfQueuedFrames() {
        TestSocket sender;
        TestSocket receiver;
        BluetoothTransfer outgoing(&sender);
        BluetoothTransfer incoming(&receiver);
        QSignalSpy received(&incoming, &BluetoothTransfer::itemReceived);
        QVERIFY(outgoing.sendFrame(attachment("file", "file.txt", "contents")));
        QVERIFY(outgoing.sendFrame(BluetoothProtocol::encodeText("after")));
        while (sender.bytesToWrite() > 0) {
            receiver.feed(sender.drain(31));
            QCoreApplication::processEvents();
        }
        QCOMPARE(received.size(), 2);
        QCOMPARE(received[0][0].toMap().value("type").toString(), QString("file"));
        QCOMPARE(received[1][0].toMap().value("data").toString(), QString("after"));
    }

    void receivedFilesAreSafeAndCopyable() {
        NetworkSwitch transport(nullptr, nullptr, nullptr);
        Backend backend(nullptr, nullptr, nullptr, &transport, nullptr, nullptr, m_directory.filePath("cache"));
        QSignalSpy history(&backend, &Backend::itemAdded);
        const QVariantMap first{{"type", "file"}, {"file_name", "../../same.txt"}, {"file_bytes", QByteArray("first")}};
        backend.handleRemoteData(first);
        QCOMPARE(history.size(), 1);
        const QVariantMap item = history[0][0].toMap();
        const QString firstPath = item.value("data").toString();
        QVERIFY(firstPath.startsWith(m_directory.filePath("cache") + '/'));
        QCOMPARE(QFileInfo(firstPath).fileName(), QString("same.txt"));
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->urls().first().toLocalFile(), firstPath);
        backend.onClipboardChanged();
        QCOMPARE(history.size(), 1);
        QVERIFY(transport.bluetoothTransferStatus().isEmpty());

        backend.handleRemoteData({{"type", "file"}, {"file_name", "C:\\elsewhere\\same.txt"}, {"file_bytes", QByteArray("second")}});
        QCOMPARE(history.size(), 2);
        const QString secondPath = history[1][0].toMap().value("data").toString();
        QVERIFY(firstPath != secondPath);
        QFile file(firstPath);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("first"));
        backend.copyToClip(item);
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->urls().first().toLocalFile(), firstPath);
        QVERIFY(!QGuiApplication::clipboard()->mimeData()->hasText()
                || QGuiApplication::clipboard()->text() != firstPath);
        QCOMPARE(ClipboardFiles::safeName("..\\CON.txt"), QString("_CON.txt"));
    }

    void receivedPhotoAndEmptyFileEnterHistory() {
        Backend backend(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_directory.filePath("cache"));
        QSignalSpy history(&backend, &Backend::itemAdded);
        backend.handleRemoteData({{"type", "image"}, {"file_name", "photo.png"}, {"img", imageBytes()}});
        QCOMPARE(history.size(), 1);
        QCOMPARE(history[0][0].toMap().value("type").toString(), QString("image"));
        QVERIFY(QGuiApplication::clipboard()->mimeData()->hasImage());
        QCOMPARE(QGuiApplication::clipboard()->image(), QImage::fromData(imageBytes()));
        backend.handleRemoteData({{"type", "file"}, {"file_name", "empty.txt"}, {"file_bytes", QByteArray()}});
        QCOMPARE(history.size(), 2);
        const QString path = history[1][0].toMap().value("data").toString();
        QVERIFY(QFileInfo(path).isFile());
        QCOMPARE(QFileInfo(path).size(), 0);
        backend.handleRemoteData({{"type", "image"}, {"img", QByteArray("broken image")}});
        QCOMPARE(history.size(), 2);
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->urls().first().toLocalFile(), path);
    }

    void copiesMultipleFilesFromHistory() {
        Backend backend(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_directory.filePath("cache"));
        QSignalSpy history(&backend, &Backend::itemAdded);
        backend.handleRemoteData({{"type", "files"}, {"files", QVariantList{
            QVariantMap{{"type", "file"}, {"file_name", "one.txt"}, {"file_bytes", QByteArray("one")}},
            QVariantMap{{"type", "file"}, {"file_name", "two.txt"}, {"file_bytes", QByteArray("two")}}
        }}});
        QCOMPARE(history.size(), 1);
        QVariantMap item = history[0][0].toMap();
        QCOMPARE(item.value("type").toString(), QString("files"));
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->urls().size(), 2);
        QGuiApplication::clipboard()->clear();
        item.remove("paths"); // QML carries the stable JSON string role.
        backend.copyToClip(item);
        QCOMPARE(QGuiApplication::clipboard()->mimeData()->urls().size(), 2);
    }

    void noEchoWithHistoryDisabledAndLocalCopyIsNotLost() {
        NetworkSwitch transport(nullptr, nullptr, nullptr);
        BluetoothServer server;
        BluetoothClient client;
        Backend backend(&server, nullptr, &client, &transport, nullptr, nullptr, m_directory.filePath("cache"));
        backend.setSaveClipboardHistory(false);
        QSignalSpy history(&backend, &Backend::itemAdded);
        server.dataReceived({{"type", "file"}, {"file_name", "remote.txt"}, {"file_bytes", QByteArray("remote")}});
        backend.onClipboardChanged();
        QVERIFY(transport.bluetoothTransferStatus().isEmpty());
        client.dataReceived({{"type", "image"}, {"file_name", "remote.png"}, {"img", imageBytes()}});
        backend.onClipboardChanged();
        QVERIFY(transport.bluetoothTransferStatus().isEmpty());
        backend.processExternalText("local copy immediately after remote");
        QVERIFY(transport.bluetoothTransferStatus().contains("Waiting"));
        QCOMPARE(history.size(), 0);
    }

    void excludingImagesDoesNotExcludeFiles() {
        Backend backend(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_directory.filePath("cache"));
        backend.setExcludeImages(true);
        QSignalSpy history(&backend, &Backend::itemAdded);
        backend.processExternalFiles({writeFile("photo.png", imageBytes())});
        QCOMPARE(history.size(), 0);
        backend.processExternalFiles({writeFile("document.txt", "file")});
        QCOMPARE(history.size(), 1);
        QCOMPARE(history[0][0].toMap().value("type").toString(), QString("file"));
    }
};

QTEST_MAIN(BluetoothTransferTest)
#include "tst_bluetooth_transfer.moc"
