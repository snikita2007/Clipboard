#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent){
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_cacheDir = tempPath + "/ClipboardHistory_Cache";
    QDir().mkpath(m_cacheDir);

    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);

    m_network = new NetworkManager(this);
    connect(m_network, &NetworkManager::dataReceived, this, &Backend::handleRemoteData);

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
        
        // 1. Устанавливаем URL (адрес на компе)
        // Это самое важное: onCC увидит это как hasUrls() и хэш совпадет
        mimeData->setUrls({ QUrl::fromLocalFile(path) });

        // 2. Добавляем ImageData как "второй слой"
        // Это нужно только для Telegram, чтобы он видел картинку, а не иконку
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

bool Backend::isDublicate(const QMimeData *mimeData, QVariantMap &item){
    QString hash;
    if (mimeData->hasUrls()) {
        hash = mimeData->urls().first().toLocalFile();
        if (hash.contains(m_cacheDir)) {
            return true; 
        }
    }
    else if (mimeData->hasImage()){
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        if (img.isNull())
            return false;

        qint64 totalSize = img.sizeInBytes();
        int sampleSize = qMin(totalSize, (qint64)4096);

        QByteArray data;
        data.append(reinterpret_cast<const char *>(&totalSize), sizeof(totalSize));
        data.append(reinterpret_cast<const char *>(img.bits()), sampleSize);

        hash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    }
    else{
        hash = mimeData->text();
    }
    for (const QVariant &v : m_history){
        if (v.toMap()["hash"].toString() == hash)
            return true;
    }
    item["hash"] = hash;
    return false;
}

void Backend::onClipboardChanged(){
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QVariantMap item;
    QString path;

    if (isDublicate(mimeData, item))
        return;

    if (mimeData->hasUrls()){
        path = mimeData->urls().first().toLocalFile();
        item["type"] = "image";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm");
        item["data"] = path;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasImage()){
        QString hash = item["hash"].toString();
        QString path = m_cacheDir + QString("/clip_%1.png").arg(hash);

        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");

        QFile file(path);
        if (file.open(QIODevice::WriteOnly)){
            file.write(data);
            file.close();
        }

        item["type"] = "image";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm");
        item["data"] = path;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasText()){
        QString text = mimeData->text();
        item["type"] = "text";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm");
        item["data"] = text;
        item["preview"] = text.simplified();
    }

    if (!item.isEmpty()){
        m_history.prepend(item);
        emit itemAdded(item);
        if (m_history.size() > 50)
            m_history.removeLast();

        QVariantMap networkItem = item;
        if (item["type"] == "image")
        {
            QImage img(item["data"].toString());
            QByteArray ba;
            QBuffer buf(&ba);
            img.save(&buf, "PNG");
            networkItem["raw_data"] = ba;
        }
        m_network->broadcastClipboard(networkItem);
    }
}

void Backend::clearHistory(){
    m_history.clear();
}
