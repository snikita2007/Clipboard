#include "backend.h"

Backend::Backend(BluetoothServer *bluetooth, UdpServer *server, BluetoothClient *cli, NetworkSwitch *m_networkSwitch , QObject *parent) : QObject(parent){
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_cacheDir = tempPath + "/ClipboardHistory_Cache";
    QDir().mkpath(m_cacheDir);

    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);
    
    connect(&UdpServer::instance(), &UdpServer::dataReceived, this, &Backend::handleRemoteData);

    connect(this, &Backend::itemAdded, [bluetooth, cli, m_networkSwitch](const QVariant &item) {
        QVariantMap map = item.toMap();
        
        if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Wifi) {
            
            UdpServer::instance().sayHello(map);
            qDebug() << "tcp" << map;
        } 
        
        else if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Bluetooth) {
            
            if (map["type"].toString() != "text") {
                return;
            }

            QString textToSend = map["data"].toString(); 

            if (cli && cli->isConnected()) {
                cli->sendMessage(textToSend);
            } 
            else if (bluetooth && bluetooth->hasActiveConnections()) {
                bluetooth->sendMessage(textToSend);
            }
        }
    });

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
    if (!m_deviceSynk) return;

    QClipboard *clipboard = QGuiApplication::clipboard();

    if (data["type"].toString() == "image"){
        QByteArray rawData = data["img"].toByteArray(); 
        
        QString hash = QCryptographicHash::hash(rawData, QCryptographicHash::Md5).toHex();
        QString path = m_cacheDir + QString("/remote_%1.png").arg(hash);

        QFile file(path);
        if (file.open(QIODevice::WriteOnly)){
            file.write(rawData);
            file.close();
            clipboard->setImage(QImage(path));
        }
    }
    else{
        clipboard->setText(data["data"].toString());
    }
}

void Backend::copyToClip(QVariant itemData){
    QVariantMap map = itemData.toMap();
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString path = map["data"].toString();

    if (map["type"].toString() == "image" && QFile::exists(path)) {
        QMimeData *mimeData = new QMimeData();
        mimeData->setUrls({ QUrl::fromLocalFile(path) });

        QImage img(path);
        if (!img.isNull()) {
            mimeData->setImageData(img);
        }

        clipboard->setMimeData(mimeData);
    } 
    else{
        clipboard->setText(map["data"].toString());
    }
}

bool Backend::isDublicate(const QString &hash) {
    if (hash.contains(m_cacheDir)) return true;
    for (const QVariant &v : m_history) {
        if (v.toMap()["hash"].toString() == hash) {
            return true;
        }
    }
    return false;
}

QString Backend::generateHash(const QMimeData *mimeData){
    if (mimeData->hasUrls() && !mimeData->urls().first().toLocalFile().isEmpty()) {
        QString path = mimeData->urls().first().toLocalFile();
        QFileInfo fileInfo(path);
        QString suffix = fileInfo.suffix().toLower();
        QStringList imageExtensions = {"png", "jpg", "jpeg", "gif", "bmp", "ico", "webp"};
        
        if (imageExtensions.contains(suffix)) {
            return path;
        }
        return QString();
    }
    
    if (mimeData->hasImage()) {
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        if (img.isNull()) return QString();

        qint64 totalSize = img.sizeInBytes();
        int sampleSize = qMin(totalSize, (qint64)4096);

        QByteArray data;
        data.append(reinterpret_cast<const char *>(&totalSize), sizeof(totalSize));
        data.append(reinterpret_cast<const char *>(img.bits()), sampleSize);

        return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    }
    
    if (mimeData->hasText()) {
        return mimeData->text();
    }

    return QString();
}

void Backend::onClipboardChanged(){
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (m_excludeImages) {
        if (mimeData->hasImage()) return;
        if (mimeData->hasUrls()) {
            QString path = mimeData->urls().first().toLocalFile();
            QString suffix = QFileInfo(path).suffix().toLower();
            if (QStringList({"png", "jpg", "jpeg", "gif", "bmp", "ico", "webp"}).contains(suffix)) {
                return;
            }
        }
    }

    QString hash = generateHash(mimeData);
    if (hash.isEmpty()) return;
    if (isDublicate(hash)) return;
    QVariantMap item;
    item["hash"] = hash;
    item["time"] = QDateTime::currentDateTime().toString("HH:mm");
    item["full_datetime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    QByteArray data;

    if (mimeData->hasUrls()  && !mimeData->urls().first().toLocalFile().isEmpty()){
        QString path = mimeData->urls().first().toLocalFile();
        QFileInfo fileInfo(path);
        QString name = fileInfo.fileName();
        item["type"] = "image";
        item["data"] = path;
        item["name"] = name;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasImage()){
        QString name = QString("img_%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
        QString path = m_cacheDir + "/" + name;
        QImage img = qvariant_cast<QImage>(mimeData->imageData());

        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");

        QFile file(path);
        if (file.open(QIODevice::WriteOnly)){
            file.write(data);
            file.close();
        }

        item["type"] = "image";
        item["data"] = path;
        item["name"] = name;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasText()){
        QString text = mimeData->text();
        item["type"] = "text";
        item["data"] = text;
        item["name"] = text.simplified();
        item["preview"] = text;
    }

    if (!item.isEmpty()){
        if (m_saveClipboardHistory){
            m_history.prepend(item);
            emit itemAdded(item);
            
            int limit = 50;
            if (m_historyLimit == 1) limit = 100;
            else if (m_historyLimit == 2) limit = 250;

            while (m_history.size() > limit)
                m_history.removeLast();
        }

        enforceAutoClear();

        if (m_deviceSynk) {
            QVariantMap networkItem = item;
            if (item["type"] == "image")
            {
                if (!data.isEmpty()) {
                    networkItem["raw_data"] = data;
                } 
                else {
                    QFile file(item["data"].toString());
                    if (file.open(QIODevice::ReadOnly)) {
                        networkItem["raw_data"] = file.readAll();
                        file.close();
                    }
                }
            }
            m_network->broadcastClipboard(networkItem);

            
        }
    }
}

void Backend::clearHistory(){
    m_history.clear();
    QDir dir(m_cacheDir);
    if (dir.exists()){
        dir.removeRecursively();
    }
}

QPoint Backend::getGlobalMousePos() {
    return QCursor::pos();
}

void Backend::setLaunchAtStartup(bool enabled) {
    m_launchAtStartup = enabled;
    #if defined(Q_OS_WIN)
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        if (enabled) {
            settings.setValue(QCoreApplication::applicationName(), QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
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
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << "[Desktop Entry]\n"
                    << "Type=Application\n"
                    << "Name=" << QCoreApplication::applicationName() << "\n"
                    << "Exec=" << QCoreApplication::applicationFilePath() << "\n"
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
    m_historyLimit = index;
    int limit = 50;
    if (m_historyLimit == 1) limit = 100;
    else if (m_historyLimit == 2) limit = 250;

    while (m_history.size() > limit) {
        m_history.removeLast();
    }
}

void Backend::setSaveClipboardHistory(bool enabled) {
    m_saveClipboardHistory = enabled;
}

void Backend::setDeviceSync(bool enabled) {
    m_deviceSynk = enabled;
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

