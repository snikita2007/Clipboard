#include "backend.h"
#include "server.h"
#include "bluetooth.h"
#include "switch.h"
#include "bluetooth_client.h"

Backend::Backend(BluetoothServer *bluetooth, UdpServer *server, BluetoothClient *cli, NetworkSwitch *m_networkSwitch , QObject *parent) : QObject(parent){
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_cacheDir = tempPath + "/ClipboardHistory_Cache";
    QDir().mkpath(m_cacheDir);

    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);

    // m_network = new NetworkManager(this);
    // connect(m_network, &NetworkManager::dataReceived, this, &Backend::handleRemoteData);

    connect(this, &Backend::itemAdded, [bluetooth, server, cli, m_networkSwitch](const QVariant &item) {
        QVariantMap map = item.toMap();
        QString text = map["text"].toString();

        if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Wifi) {
            
            server->sayHello(map);
            qDebug() << "TCP SEND";

        } else if (m_networkSwitch->currentMode() == NetworkSwitch::TransportMode::Bluetooth) {
            
            if (cli && cli->isConnected()) {
                cli->sendMessage(text);
            } else if (bluetooth && bluetooth->hasActiveConnections()) {
                bluetooth->sendMessage(text);
            }
            qDebug() << "Bluetooth SEND";
            
        }
    });


    // QObject::connect(this, &Backend::itemAdded, [bluetooth, server, cli](const QVariant &item) {
    //     QVariantMap map = item.toMap();
    //     server->sayHello(map);

    //     if (cli->isConnected()) { 
    //         cli->sendMessage("TEST");
    //     } 

    //     else if (bluetooth->hasActiveConnections()) {
    //         bluetooth->sendMessage("TEST");
    //     }
    // });
    // m_network = new NetworkManager(this);
    // connect(m_network, &NetworkManager::dataReceived, this, &Backend::handleRemoteData);

// Таймер для обновления буфера на мак
#ifdef Q_OS_MAC
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Backend::checkClipboardNative);
    timer->start(500);
#endif
}

Backend::~Backend(){
    // Отчистка папки в temp
    QDir dir(m_cacheDir);
    if (dir.exists()){
        dir.removeRecursively();
        qDebug() << "Cleaned up...";
    }
}

void Backend::handleRemoteData(QVariantMap data){
    QClipboard *clipboard = QGuiApplication::clipboard();

    if (data["type"].toString() == "image"){
        QByteArray rawData = data["raw_data"].toByteArray();
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


    // 2. Отправляем в сеть (если нужно принудительно при клике)
    // Подготовим данные: если это картинка, нужно отправить байты, а не путь
    if (map["type"].toString() == "image") {
        QImage img(map["data"].toString());
        QByteArray ba;
        QBuffer buf(&ba);
        img.save(&buf, "JPG");
        map["raw_data"] = ba; // Добавляем байты для сети
    }

    // m_network->broadcastClipboard(map);

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

    QString hash = generateHash(mimeData);
    if (hash.isEmpty()) return;
    if (isDublicate(hash)) return;
    QVariantMap item;
    item["hash"] = hash;
    item["time"] = QDateTime::currentDateTime().toString("HH:mm");
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
        m_history.prepend(item);
        emit itemAdded(item);
        
        if (m_history.size() > 50)
            m_history.removeLast();

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

        // qDebug() << "SENDING:" << networkItem["type"].toString() << "Data size:" << networkItem["data"].toString().length();
        // m_network->broadcastClipboard(networkItem);

    }
}

void Backend::clearHistory(){
    m_history.clear();
}

QPoint Backend::getGlobalMousePos() {
    return QCursor::pos();
}
