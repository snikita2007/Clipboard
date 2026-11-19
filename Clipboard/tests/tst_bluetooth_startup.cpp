#include <QtTest>
#include <QSettings>
#include <QTemporaryDir>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <memory>

#include "bluetooth_reconnect.h"
#include "switch.h"

namespace {
const QString phone = QStringLiteral("10:20:30:40:50:60");
const QString laptop = QStringLiteral("AA:BB:CC:DD:EE:FF");
}

class BluetoothStartupTest : public QObject
{
    Q_OBJECT
    QTemporaryDir m_settingsDir;

private slots:
    void initTestCase()
    {
        QVERIFY(m_settingsDir.isValid());
        QCoreApplication::setOrganizationName("ClipboardTests");
        QCoreApplication::setApplicationName("BluetoothStartup");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, m_settingsDir.path());
    }

    void init() { QSettings().clear(); }

    void startupMode_data()
    {
        QTest::addColumn<bool>("automatic");
        QTest::addColumn<int>("savedTransport");
        QTest::addColumn<int>("expected");
        QTest::newRow("automatic-overrides-wifi") << true << 0 << 1;
        QTest::newRow("automatic-bluetooth") << true << 1 << 1;
        QTest::newRow("manual-wifi") << false << 0 << 0;
        QTest::newRow("manual-bluetooth") << false << 1 << 1;
    }

    void startupMode()
    {
        QFETCH(bool, automatic);
        QFETCH(int, savedTransport);
        QFETCH(int, expected);
        QSettings().setValue("ApplicationSettings/bluetoothOnStartup", automatic);
        QSettings().setValue("ApplicationSettings/transportMode", savedTransport);
        NetworkSwitch transport(nullptr, nullptr, nullptr);
        QCOMPARE(transport.transportIndex(), expected);
    }

    void startupDefaultsToBluetooth()
    {
        NetworkSwitch transport(nullptr, nullptr, nullptr);
        QVERIFY(transport.bluetoothOnStartup());
        QCOMPARE(transport.currentMode(), NetworkSwitch::Bluetooth);
        transport.setBluetoothOnStartup(false);
        transport.setTransport(0);
        NetworkSwitch restarted(nullptr, nullptr, nullptr);
        QVERIFY(!restarted.bluetoothOnStartup());
        QCOMPARE(restarted.currentMode(), NetworkSwitch::Wifi);
    }

    void remembersIncomingConnectionWithoutDiscovery()
    {
        {
            BluetoothServer server;
            BluetoothClient client;
            NetworkSwitch transport(nullptr, &server, &client);
            server.peerConnected(phone, "Phone");
        }
        BluetoothClient client;
        NetworkSwitch restarted(nullptr, nullptr, &client);
        QCOMPARE(client.devices().size(), 1);
        QCOMPARE(client.devices().first().toMap().value("address").toString(), phone);
        QCOMPARE(client.devices().first().toMap().value("name").toString(), QString("Phone"));
        QVERIFY(!client.isDiscovering());
    }

    void restoresPeerAndWaitsForAdapter()
    {
        {
            BluetoothReconnect previousSession;
            previousSession.rememberDevice(phone, "Phone");
        }
        BluetoothReconnect reconnect;
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        QCOMPARE(reconnect.lastAddress(), phone);
        QCOMPARE(reconnect.lastName(), QString("Phone"));
        reconnect.setEnabled(true);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 0);
        reconnect.setReady(true);
        QTRY_COMPARE(attempts.count(), 1);
        QCOMPARE(attempts.first().first().toString(), phone);
        reconnect.updateConnectionState(false, true);
        reconnect.setReady(true);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 1);
    }

    void retriesUnavailablePeer()
    {
        BluetoothReconnect reconnect;
        reconnect.rememberDevice(phone, "Phone");
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        QTRY_COMPARE(attempts.count(), 1);
        reconnect.updateConnectionState(false, false);
        QTRY_COMPARE_WITH_TIMEOUT(attempts.count(), 2, 6500);
        QCOMPARE(attempts.last().first().toString(), phone);
        reconnect.updateConnectionState(true, false);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 2);
    }

    void incomingConnectionCancelsAttemptAndReconnectsAfterLoss()
    {
        BluetoothReconnect reconnect;
        reconnect.rememberDevice(phone, "Phone");
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        reconnect.updateConnectionState(true, false);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 0);
        reconnect.updateConnectionState(false, false);
        QTRY_COMPARE_WITH_TIMEOUT(attempts.count(), 1, 6500);
    }

    void cancellation_data()
    {
        QTest::addColumn<int>("reason");
        QTest::newRow("switch-to-wifi") << 0;
        QTest::newRow("adapter-not-ready") << 1;
        QTest::newRow("manual-disconnect") << 2;
    }

    void cancellation()
    {
        QFETCH(int, reason);
        BluetoothReconnect reconnect;
        reconnect.rememberDevice(phone, "Phone");
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        if (reason == 0) reconnect.setEnabled(false);
        if (reason == 1) reconnect.setReady(false);
        if (reason == 2) reconnect.pause();
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 0);
    }

    void manualDisconnectPersistsUntilNextSession()
    {
        BluetoothReconnect reconnect;
        reconnect.rememberDevice(phone, "Phone");
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        QTRY_COMPARE(attempts.count(), 1);
        reconnect.updateConnectionState(true, false);
        reconnect.pause();
        reconnect.updateConnectionState(false, false);
        reconnect.setReady(false);
        reconnect.setReady(true);
        QTest::qWait(100);
        QCOMPARE(attempts.count(), 1);

        BluetoothReconnect restarted;
        QSignalSpy restartedAttempts(&restarted, &BluetoothReconnect::connectRequested);
        restarted.setEnabled(true);
        restarted.setReady(true);
        QTRY_COMPARE(restartedAttempts.count(), 1);
        QCOMPARE(restartedAttempts.first().first().toString(), phone);
    }

    void manualChoiceDoesNotRetryOldPeer()
    {
        BluetoothReconnect reconnect;
        reconnect.rememberDevice(phone, "Phone");
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        reconnect.pause();
        reconnect.updateConnectionState(false, true);
        reconnect.updateConnectionState(false, false);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 0);
        QCOMPARE(BluetoothReconnect().lastAddress(), phone);
        reconnect.rememberDevice(laptop, "Laptop");
        reconnect.updateConnectionState(true, false);
        QCOMPARE(BluetoothReconnect().lastAddress(), laptop);
        QCOMPARE(BluetoothReconnect().lastName(), QString("Laptop"));
    }

    void ignoresInvalidSavedAddresses_data()
    {
        QTest::addColumn<QString>("address");
        QTest::newRow("first-launch") << QString();
        QTest::newRow("corrupt-setting") << QString("invalid");
        QTest::newRow("null-address") << QString("00:00:00:00:00:00");
    }

    void ignoresInvalidSavedAddresses()
    {
        QFETCH(QString, address);
        QSettings().setValue("Bluetooth/lastDeviceAddress", address);
        BluetoothReconnect reconnect;
        QSignalSpy attempts(&reconnect, &BluetoothReconnect::connectRequested);
        reconnect.setEnabled(true);
        reconnect.setReady(true);
        QTest::qWait(50);
        QCOMPARE(attempts.count(), 0);
        QVERIFY(reconnect.lastAddress().isEmpty());
        reconnect.rememberDevice(phone, "Phone");
        reconnect.rememberDevice(address, "Invalid");
        QCOMPARE(BluetoothReconnect().lastAddress(), phone);
    }

    void settingsPageReflectsStartupMode()
    {
        QSettings().setValue("ApplicationSettings/transportMode", 0);
        QSettings().setValue("ApplicationSettings/historyLimit", 10);
        NetworkSwitch transport(nullptr, nullptr, nullptr);
        QQmlEngine engine;
        engine.rootContext()->setContextProperty("networkSwitch", &transport);
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(CLIPBOARD_SOURCE_DIR "/ui/Settings.qml")));
        std::unique_ptr<QObject> page(component.create());
        QVERIFY2(page, qPrintable(component.errorString()));
        QObject *connectionChoice = nullptr;
        QObject *startupToggle = nullptr;
        for (QObject *child : page->findChildren<QObject *>()) {
            if (child->property("title").toString() == "Connection") connectionChoice = child;
            if (child->property("title").toString() == "Bluetooth on startup") startupToggle = child;
        }
        QVERIFY(connectionChoice);
        QVERIFY(startupToggle);
        QCOMPARE(transport.transportIndex(), 1);
        QCOMPARE(page->property("transportMode").toInt(), 1);
        QCOMPARE(connectionChoice->property("currentIndex").toInt(), 1);
        QVERIFY(QMetaObject::invokeMethod(connectionChoice, "activated", Q_ARG(int, 0)));
        QCOMPARE(transport.transportIndex(), 0);
        QCOMPARE(page->property("transportMode").toInt(), 0);
        transport.setTransport(1);
        QCOMPARE(connectionChoice->property("currentIndex").toInt(), 1);
        transport.setBluetoothOnStartup(false);
        QVERIFY(!startupToggle->property("checked").toBool());
        QVERIFY(QMetaObject::invokeMethod(startupToggle, "toggled", Q_ARG(bool, true)));
        QVERIFY(transport.bluetoothOnStartup());
    }
};

QTEST_MAIN(BluetoothStartupTest)
#include "tst_bluetooth_startup.moc"
