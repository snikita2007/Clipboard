#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) 
{   
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

Backend::~Backend() {
    // Отчистка папки в temp
    QDir dir(m_cacheDir);
    if (dir.exists()) {
        dir.removeRecursively(); 
        qDebug() << "Cleaned up...";
    }
}

void Backend::handleRemoteData(QVariantMap data) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    
    if (data["type"].toString() == "image") {
        QByteArray rawData = data["raw_data"].toByteArray();
        QString hash = QCryptographicHash::hash(rawData, QCryptographicHash::Md5).toHex();
        QString path = m_cacheDir + QString("/remote_%1.png").arg(hash);
        
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(rawData);
            file.close();
            clipboard->setImage(QImage(path));
        }
    } else {
        clipboard->setText(data["data"].toString());
    }
}

void Backend::copyToClip(QVariant itemData) {
    QVariantMap map = itemData.toMap();
    QClipboard *clipboard = QGuiApplication::clipboard();
    
    // 1. Устанавливаем в локальный буфер
    if (map["type"].toString() == "image") {
        clipboard->setImage(QImage(map["data"].toString()));
    } else {
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
    m_network->broadcastClipboard(map);
}


void Backend::onClipboardChanged(){
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QVariantMap item;
    QString hash;
    QString path;

    if (mimeData->hasImage()){
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
        buffer.close();
        hash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
        path = m_cacheDir + QString("/clip_%1.png").arg(hash);
        for (const QVariant &v : m_history){
            if (v.toMap()["hash"].toString() == hash) return;
        }
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }
        item["type"] = "image";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm"); 
        item["data"] = path;
        item["hash"] = hash;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }
    
    else if (mimeData->hasUrls()){
        path = mimeData->urls().first().toLocalFile();
        for (const QVariant &v : m_history){
            if (v.toMap()["data"].toString() == path) return;
        }
        item["type"] = "image";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm"); 
        item["data"] = path;
        item["preview"] = QUrl::fromLocalFile(path).toString();
    }

    else if (mimeData->hasText()){
        QString text = mimeData->text();
        for (const QVariant &v : m_history) {
            if (v.toMap()["data"].toString() == text) return;
        }
        item["type"] = "text";
        item["time"] = QDateTime::currentDateTime().toString("HH:mm"); 
        item["data"] = text;
        item["preview"] = text.simplified();
    }

    if (!item.isEmpty()) {
        m_history.prepend(item);
        emit itemAdded(item);
        if (m_history.size() > 50) m_history.removeLast();

        QVariantMap networkItem = item;
        if (item["type"] == "image") {
            QImage img(item["data"].toString());
            QByteArray ba;
            QBuffer buf(&ba);
            img.save(&buf, "PNG");
            networkItem["raw_data"] = ba;
        }
        qDebug() << "SENDING:" << networkItem["type"].toString() << "Data size:" << networkItem["data"].toString().length();
        m_network->broadcastClipboard(networkItem);
    }
}

void Backend::clearHistory() {
    m_history.clear();  
}






