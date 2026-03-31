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
            mimeData->setImageData(img);
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

void Backend::onClipboardChanged() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (!mimeData) return;

    QVariantMap item;
    QString currentHash;
    QString finalPath;

    if (mimeData->hasUrls() && !mimeData->urls().isEmpty()) {
        finalPath = mimeData->urls().first().toLocalFile();
        if (!finalPath.isEmpty()) {
            QFile file(finalPath);
            if (file.open(QIODevice::ReadOnly)) {
                currentHash = QCryptographicHash::hash(file.read(5 * 1024 * 1024), QCryptographicHash::Md5).toHex();
                file.close();
            }
        }
    }

    if (currentHash.isEmpty() && mimeData->hasImage()) {
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        if (!img.isNull()) {
            QByteArray jpgData;
            QBuffer buffer(&jpgData);
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, "JPG", 90); 
            buffer.close();

            currentHash = QCryptographicHash::hash(jpgData, QCryptographicHash::Md5).toHex();
            finalPath = m_cacheDir + QString("/clip_%1.jpg").arg(currentHash);

            if (!QFile::exists(finalPath)) {
                QFile file(finalPath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(jpgData);
                    file.close();
                }
            }
        }
    }

    if (!currentHash.isEmpty()) {
        for (const QVariant &v : m_history) {
            if (v.toMap()["hash"].toString() == currentHash) return; 
        }
        item["type"] = "image";
        item["data"] = finalPath;
        item["hash"] = currentHash;
        item["preview"] = QUrl::fromLocalFile(finalPath).toString();
    } 

    else if (mimeData->hasText()) {
        QString text = mimeData->text().trimmed();
        if (text.isEmpty()) return;

        for (const QVariant &v : m_history) {
            if (v.toMap()["data"].toString() == text) return;
        }

        item["type"] = "text";
        item["data"] = text;
        item["preview"] = text.left(100).simplified();
    }

    if (!item.isEmpty()) {
        m_history.prepend(item);
        emit itemAdded(item);
        if (m_history.size() > 50) m_history.removeLast();
    }
}

void Backend::clearHistory() {
    m_history.clear();  
}






