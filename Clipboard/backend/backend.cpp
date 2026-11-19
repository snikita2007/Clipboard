#include "backend.h"
#include "clipboard_files.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <memory>
#include <utility>

#ifdef Q_OS_ANDROID
#include "android_clipboard.h"
#endif

Backend::Backend(BluetoothServer *bluetooth, UdpServer *server, BluetoothClient *cli, NetworkSwitch *networkSwitch, NetworkScanner *networkScanner, QObject *parent, const QString &cacheDir)
    : QObject(parent), m_bluetooth(bluetooth), m_btClient(cli), m_networkSwitch(networkSwitch) {
    Q_UNUSED(server);
    Q_UNUSED(networkScanner);

    m_cacheDir = cacheDir.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/clipboard-history" : cacheDir;
    QDir().mkpath(m_cacheDir);

    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);

    connect(&UdpServer::instance(), &UdpServer::dataReceived, this, &Backend::handleRemoteData);

    if (m_bluetooth) {
        connect(m_bluetooth, &BluetoothServer::dataReceived, this, &Backend::handleRemoteData);
        connect(m_bluetooth, QOverload<const QString &>::of(&BluetoothServer::clientConnected),
                this, &Backend::sendPendingBluetooth);
    }
    if (m_btClient) {
        connect(m_btClient, &BluetoothClient::dataReceived, this, &Backend::handleRemoteData);
        connect(m_btClient, &BluetoothClient::connected, this, &Backend::sendPendingBluetooth);
    }
    if (m_networkSwitch) {
        connect(m_networkSwitch, &NetworkSwitch::modeChanged, this, [this]() { m_pendingBluetoothFrame.clear(); });
    }

    #ifdef Q_OS_MAC
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Backend::checkClipboardNative);
    timer->start(500);
    #endif
    #ifdef Q_OS_ANDROID
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            QTimer::singleShot(300, this, &Backend::onClipboardChanged);
        }
    });
    #endif
}

Backend::~Backend(){
    if (m_clearOnExit) {
        QDir dir(m_cacheDir);
        if (dir.exists()){
            dir.removeRecursively();
        }
    }
}

void Backend::handleRemoteData(QVariantMap data){
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QString type = data.value("type").toString();
    auto mimeData = std::make_unique<QMimeData>();
    QStringList paths;
    if (type == "text") {
        mimeData->setText(data.value("data").toString());
    } else if (type == "image" || type == "file" || type == "files") {
        const QVariantList files = type == "files" ? data.value("files").toList() : QVariantList{data};
        if (files.isEmpty() || files.size() > BluetoothProtocol::MaxFiles) return;
        QList<QUrl> urls;
        for (const QVariant &value : files) {
            const QVariantMap file = value.toMap();
            const bool isImage = file.value("type").toString() == "image";
            const QString key = isImage ? "img" : "file_bytes";
            if ((!isImage && file.value("type").toString() != "file") || !file.contains(key)) return;
            const QByteArray bytes = file.value(key).toByteArray();
            if (isImage) {
                const QImage image = QImage::fromData(bytes);
                if (image.isNull()) return;
                if (files.size() == 1) mimeData->setImageData(image);
            }
            const QString name = file.value("file_name", isImage ? "image.png" : "attachment").toString();
            const QString path = ClipboardFiles::store(m_cacheDir, name, bytes);
            if (path.isEmpty()) {
                if (m_networkSwitch) m_networkSwitch->reportBluetoothTransfer(tr("Cannot save the received file."));
                return;
            }
            paths.append(path);
            urls.append(QUrl::fromLocalFile(path));
        }
        mimeData->setUrls(urls);
    } else {
        return;
    }

    m_lastRemoteHash = generateHash(mimeData.get());
    m_pendingBluetoothFrame.clear();
    // Add all types immediately: Android may not emit a clipboard signal in the background.
    processMimeData(mimeData.get(), true);
#ifdef Q_OS_ANDROID
    AndroidClipboard::markOwnClipboardWrite();
    if (!paths.isEmpty()) {
        AndroidClipboard::setClipboardFiles(paths);
        return;
    }
#endif
    clipboard->setMimeData(mimeData.release());
}

void Backend::processExternalText(const QString &text){
    if (text.trimmed().isEmpty()) return;

    QMimeData mimeData;
    mimeData.setText(text);
    processMimeData(&mimeData, false);
}

void Backend::processExternalFiles(const QStringList &paths){
    if (paths.isEmpty()) return;
    QMimeData mimeData;
    QList<QUrl> urls;
    for (const QString &path : paths) urls.append(QUrl::fromLocalFile(path));
    mimeData.setUrls(urls);
    processMimeData(&mimeData, false);
}

QString Backend::backgroundSendStatus(){
#ifdef Q_OS_ANDROID
    const QString status = AndroidClipboard::backgroundStatus();
    if (status == "auto")
        return tr("Copied text, photos and small files are sent automatically, even when the app is in the background.");
    if (status == "no-overlay")
        return tr("Allow \"Display over other apps\" (on Xiaomi also \"Display pop-up windows while running in the background\") to send clipboard content automatically.");
    return tr("In the background, tap \"Send clipboard\" in the notification or use Share → Clipboard sync. "
              "Automatic sending needs the READ_LOGS permission, which can be given only over adb.");
#else
    return QString();
#endif
}

void Backend::copyToClip(QVariant itemData){
    QVariantMap map = itemData.toMap();
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString path = map["data"].toString();

    const QString type = map.value("type").toString();
    if (type == "image" || type == "file" || type == "files") {
        QStringList paths = type == "files" ? map.value("paths").toStringList() : QStringList{path};
        if (type == "files" && paths.isEmpty()) {
            const QJsonArray array = QJsonDocument::fromJson(map.value("pathsJson").toString().toUtf8()).array();
            for (const QJsonValue &value : array) paths.append(value.toString());
        }
        if (paths.isEmpty()) return;
        QList<QUrl> urls;
        for (const QString &filePath : paths) {
            if (!QFileInfo(filePath).isFile()) return;
            urls.append(QUrl::fromLocalFile(filePath));
        }
#ifdef Q_OS_ANDROID
        AndroidClipboard::setClipboardFiles(paths);
        return;
#endif
        QMimeData *mimeData = new QMimeData();
        mimeData->setUrls(urls);

        if (type == "image") {
            QImage img(path);
            if (!img.isNull()) mimeData->setImageData(img);
        }

        clipboard->setMimeData(mimeData);
    } 
    else{
        clipboard->setText(map["data"].toString());
    }
}

bool Backend::isDublicate(const QString &hash) {
    for (const QVariant &v : m_history) {
        if (v.toMap()["hash"].toString() == hash) {
            return true;
        }
    }
    return false;
}

QString Backend::generateHash(const QMimeData *mimeData){
    if (mimeData->hasUrls() && !mimeData->urls().isEmpty() && mimeData->urls().first().isLocalFile()) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        for (const QUrl &url : mimeData->urls()) {
            const QFileInfo fileInfo(url.toLocalFile());
            if (!url.isLocalFile() || !fileInfo.isFile()) return {};
            const QString fingerprint = fileInfo.absoluteFilePath() + '\n' + QString::number(fileInfo.size())
                + '\n' + QString::number(fileInfo.lastModified().toMSecsSinceEpoch()) + '\n';
            hash.addData(fingerprint.toUtf8());
        }
        return QString::fromLatin1(hash.result().toHex());
    }
    
    if (mimeData->hasImage()) {
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        if (img.isNull()) return QString();

        img = img.convertToFormat(QImage::Format_ARGB32);
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(QByteArray::number(img.width()) + 'x' + QByteArray::number(img.height()) + ':');
        hash.addData(QByteArrayView(reinterpret_cast<const char *>(img.constBits()), img.sizeInBytes()));
        return QString::fromLatin1(hash.result().toHex());
    }
    
    if (mimeData->hasText()) {
        return mimeData->text();
    }

    return QString();
}

void Backend::onClipboardChanged(){
    QClipboard *clipboard = QGuiApplication::clipboard();
#ifdef Q_OS_ANDROID
    const QMimeData *mime = clipboard->mimeData();
    if (mime && mime->hasUrls() && !mime->urls().isEmpty() && mime->urls().first().scheme() == "content") {
        AndroidClipboard::readClipboardFiles();
        return;
    }
#endif
    processMimeData(clipboard->mimeData(), false);
}

void Backend::processMimeData(const QMimeData *mimeData, bool fromRemote){
    if (!mimeData) return;

    QString hash = generateHash(mimeData);
    if (hash.isEmpty()) return;
    QVariantMap item;
    item["hash"] = hash;
    item["time"] = QDateTime::currentDateTime().toString("HH:mm");
    item["full_datetime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    item["pathsJson"] = QString();
    QByteArray data;

    if (mimeData->hasUrls() && !mimeData->urls().isEmpty() && mimeData->urls().first().isLocalFile()) {
        QStringList paths;
        QStringList names;
        for (const QUrl &url : mimeData->urls()) {
            if (m_excludeImages && ClipboardFiles::isImage(url.toLocalFile())) continue;
            paths.append(url.toLocalFile());
            names.append(QFileInfo(url.toLocalFile()).fileName());
        }
        if (paths.isEmpty()) return;
        item["type"] = paths.size() > 1 ? "files" : (ClipboardFiles::isImage(paths.first()) ? "image" : "file");
        item["paths"] = paths;
        item["pathsJson"] = QString::fromUtf8(QJsonDocument(QJsonArray::fromStringList(paths)).toJson(QJsonDocument::Compact));
        item["data"] = paths.first();
        item["name"] = names.join(", ");
        item["preview"] = QUrl::fromLocalFile(paths.first()).toString();
    }

    else if (mimeData->hasImage()){
        if (m_excludeImages) return;
        QString name = "image.png";
        QImage img = qvariant_cast<QImage>(mimeData->imageData());

        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if (!img.save(&buffer, "PNG")) return;
        const QString path = ClipboardFiles::store(m_cacheDir, name, data);
        if (path.isEmpty()) return;

        item["type"] = "image";
        item["data"] = path;
        item["name"] = name;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasText()){
        QString text = mimeData->text();
        if (text.trimmed().isEmpty()) return;
        item["type"] = "text";
        item["data"] = text;
        item["name"] = text.simplified();
        item["preview"] = text;
    }

    const bool fromOtherDevice = fromRemote
        || hash == m_lastRemoteHash;
    if (!fromOtherDevice) m_lastRemoteHash.clear();

    if (m_saveClipboardHistory && !isDublicate(hash)){
        m_history.prepend(item);
        emit itemAdded(item);

        while (m_history.size() > qMax(m_historyLimit, 1))
            m_history.removeLast();
    }

    if (!fromOtherDevice) {
        broadcastItem(item);
    }

    enforceAutoClear();
}

void Backend::broadcastItem(const QVariantMap &item){
    if (!main_flag || !m_deviceSynk || !m_networkSwitch) return;

    if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Wifi) {
        // UdpServer lives in its own thread; run sayHello there
        QMetaObject::invokeMethod(&UdpServer::instance(), "sayHello", Qt::QueuedConnection, Q_ARG(QVariantMap, item));
    }
    else if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Bluetooth) {
        QString error;
        const QByteArray frame = BluetoothProtocol::encodeItem(item, &error);
        if (frame.isEmpty()) {
            m_pendingBluetoothFrame.clear();
            m_networkSwitch->reportBluetoothTransfer(error);
            return;
        }
        m_pendingBluetoothFrame = frame;
        if (!m_networkSwitch->bluetoothConnected())
            m_networkSwitch->reportBluetoothTransfer(tr("Clipboard is ready. Waiting for a Bluetooth connection..."));
        sendPendingBluetooth();
    }
}

void Backend::sendPendingBluetooth(){
    if (m_pendingBluetoothFrame.isEmpty() || !main_flag || !m_deviceSynk || !m_networkSwitch
            || !m_networkSwitch->bluetoothConnected()) return;
    const QByteArray frame = std::exchange(m_pendingBluetoothFrame, {});
    m_networkSwitch->reportBluetoothTransfer(tr("Sending clipboard over Bluetooth..."));
    if (m_btClient && m_btClient->isConnected()) m_btClient->sendFrame(frame);
    if (m_bluetooth && m_bluetooth->hasActiveConnections()) m_bluetooth->sendFrame(frame);
}

void Backend::clearHistory(){
    m_history.clear();
    m_pendingBluetoothFrame.clear();
    QDir dir(m_cacheDir);
    if (dir.exists()){
        dir.removeRecursively();
    }
    // The folder is needed again for the next copied image
    QDir().mkpath(m_cacheDir);
}

QPoint Backend::getGlobalMousePos() {
    return QCursor::pos();
}

#if defined(Q_OS_LINUX)
// Quotes a program path for the Exec key of a desktop entry: the path may contain spaces.
static QString desktopEntryExec(const QString &path) {
    QString quoted;
    for (const QChar c : path) {
        if (c == '\\') quoted += QStringLiteral("\\\\\\\\");
        else if (c == '"' || c == '`' || c == '$') quoted += QStringLiteral("\\\\") + c;
        else if (c == '%') quoted += QStringLiteral("%%");
        else quoted += c;
    }
    return '"' + quoted + '"';
}
#endif

void Backend::setLaunchAtStartup(bool enabled) {
    m_launchAtStartup = enabled;
    applyLaunchAtStartup(enabled);
}

void Backend::applyLaunchAtStartup(bool enabled) {
    #if defined(Q_OS_WIN)
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        if (enabled) {
            settings.setValue(QCoreApplication::applicationName(), '"' + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + '"');
        } else {
            settings.remove(QCoreApplication::applicationName());
        }
    #elif defined(Q_OS_MAC)
        QString appPath = QCoreApplication::applicationFilePath();
        QString appName = QCoreApplication::applicationName();
        QStringList args;
        if (enabled) {
            args << "-e" << QString("tell application \"System Events\" to make login item at end with properties {path:\"%1\", name:\"%2\", hidden:false}").arg(appPath, appName);
        } else {
            args << "-e" << QString("tell application \"System Events\" to delete login item \"%1\"").arg(appName);
        }
        QProcess::startDetached("osascript", args);
    #elif defined(Q_OS_LINUX)
        QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/autostart";
        QDir().mkpath(autostartDir);
        QString filePath = autostartDir + "/" + QCoreApplication::applicationName() + ".desktop";
        if (enabled) {
            // An AppImage is mounted at a new path on every start, so point to the image file itself.
            QString program = qEnvironmentVariable("APPIMAGE");
            if (program.isEmpty()) program = QCoreApplication::applicationFilePath();
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "[Desktop Entry]\n"
                    << "Type=Application\n"
                    << "Name=" << QCoreApplication::applicationName() << "\n"
                    << "Exec=" << desktopEntryExec(program) << "\n"
                    << "Hidden=false\n"
                    << "NoDisplay=false\n"
                    << "X-GNOME-Autostart-enabled=true\n";
            }
        } else {
            QFile::remove(filePath);
        }
    #endif
}

void Backend::setHistoryLimit(int index) {
    m_historyLimit = qMax(index, 1);

    while (m_history.size() > m_historyLimit) {
        m_history.removeLast();
    }
}

void Backend::setSaveClipboardHistory(bool enabled) {
    m_saveClipboardHistory = enabled;
}

void Backend::setDeviceSync(bool enabled) {
    m_deviceSynk = enabled;
    if (!enabled) m_pendingBluetoothFrame.clear();
}

void Backend::setAutoClear(int index) {
    m_autoClear = index;
    enforceAutoClear();
}

void Backend::setExcludeImages(bool enabled) {
    m_excludeImages = enabled;
}

void Backend::setClearOnExit(bool enabled) {
    m_clearOnExit = enabled;
}

void Backend::enforceAutoClear() {
    if (m_autoClear == 0 || m_history.isEmpty()) return;

    qint64 days = 1;
    if (m_autoClear == 2) days = 7;
    else if (m_autoClear == 3) days = 30;

    QDateTime limitTime = QDateTime::currentDateTime().addDays(-days);

    for (int i = m_history.size() - 1; i >= 0; --i) {
        QVariantMap item = m_history[i].toMap();
        QDateTime itemTime = QDateTime::fromString(item["full_datetime"].toString(), Qt::ISODate);
        
        if (itemTime.isValid() && itemTime < limitTime) {
            m_history.removeAt(i);
        }
    }
}

