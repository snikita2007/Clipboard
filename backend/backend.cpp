#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) 
{   
    // Создание папки с кешем в temp
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_cacheDir = tempPath + "/ClipboardHistory_Cache";
    QDir().mkpath(m_cacheDir);

    // Подключение слота к обновлению буфера
    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);

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


void Backend::copyToClip(QVariant itemData) {
    QVariantMap map = itemData.toMap();
    QClipboard *clipboard = QGuiApplication::clipboard();
    QMimeData *mimeData = new QMimeData();
    
    // Проверка типа данных
    if (map["type"].toString() == "image") {
        QImage img(map["data"].toString());
        if (!img.isNull()) {
            QList<QUrl> urls;
            urls << QUrl::fromLocalFile(map["data"].toString());
            mimeData->setUrls(urls);
        }
    } 
    else {
        mimeData->setText(map["data"].toString());
    }
    // Запись в буфер
    clipboard->setMimeData(mimeData);
}

void Backend::onClipboardChanged(){
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QVariantMap item;
    QString hash;
    QString path;

    if (mimeData->hasUrls()){
        path = mimeData->urls().first().toLocalFile();
        for (const QVariant &v : m_history){
            if (v.toMap()["data"].toString() == path){
                qDebug() << "URL skiped" << path;
                return;
            }
        }
        item["type"] = "image";
        item["data"] = path;
        item["preview"] = QUrl::fromLocalFile(path).toString();
        qDebug() << "URL copyed" << path;
    }
    
    else if (mimeData->hasImage()){
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "JPG");
        buffer.close();

        hash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
        path = m_cacheDir + QString("/clip_%1.jpg").arg(hash);
        for (const QVariant &v : m_history){
            if (v.toMap()["hash"].toString() == hash){
                qDebug() << "Img skiped" << hash;
                return;
            }
        }
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }
        item["type"] = "image";
        item["data"] = path;
        item["hash"] = hash;
        item["preview"] = QUrl::fromLocalFile(path).toString();
        qDebug() << "Img copyed" << path;
    }

    else if (mimeData->hasText()){
        QString text = mimeData->text();
        for (const QVariant &v : m_history) {
            if (v.toMap()["data"].toString() == text) {
                qDebug() << "Text skiped" << text;
                return;
            };
        }
        item["type"] = "text";
        item["data"] = text;
        item["preview"] = text.simplified();
        qDebug() << "Text copyed";
    }

    if (item.isEmpty()) return;
    m_history.prepend(item);
    emit itemAdded(item);
    if (m_history.size() > 50) m_history.removeLast();
}

void Backend::clearHistory() {
    m_history.clear();  
}






