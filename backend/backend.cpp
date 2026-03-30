#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) 
{
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_cacheDir = tempPath + "/ClipboardHistory_Cache";
    QDir().mkpath(m_cacheDir);

    QClipboard *clipboard = QGuiApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &Backend::onClipboardChanged);

    #ifdef Q_OS_MAC
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Backend::checkClipboardNative);
    timer->start(500);
    #endif
}

Backend::~Backend() {
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

    if (map["type"].toString() == "image") {
        QString path = map["data"].toString();
        QImage img(path);
        if (!img.isNull()) {
            mimeData->setImageData(img);
            QList<QUrl> urls;
            urls << QUrl::fromLocalFile(path);
            mimeData->setUrls(urls);
        }
    } 
    else {
        mimeData->setText(map["data"].toString());
    }
    clipboard->setMimeData(mimeData);
}

void Backend::onClipboardChanged() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QVariantMap item;

    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString localPath = urls.first().toLocalFile();
            if (!localPath.isEmpty()) {
                QImage img(localPath);
                if (!img.isNull()) {
                    item["type"] = "image";
                    item["data"] = localPath;
                    item["preview"] = QUrl::fromLocalFile(localPath).toString();
                }
            }
        }
    } 

    if (item.isEmpty() && mimeData->hasImage()) {
        QImage img = qvariant_cast<QImage>(mimeData->imageData());
        if (!img.isNull()) {
            for (const QVariant &v : m_history) {
                QVariantMap existing = v.toMap();
                if (existing["type"] == "image") {
                    break; 
                }
            }

            QString path = m_cacheDir + QString("/clip_%1.jpg").arg(QDateTime::currentMSecsSinceEpoch());
            QImage scaledImg = img.scaled(2000, 2000, Qt::KeepAspectRatio, Qt::FastTransformation);
            if (scaledImg.save(path, "JPG", 85)) {
                item["type"] = "image";
                item["data"] = path;
                item["preview"] = QUrl::fromLocalFile(path).toString();
            }
        }
    }

    if (item.isEmpty() && mimeData->hasText()) {
        QString text = mimeData->text();
        if (text.trimmed().isEmpty()) return;
        item["type"] = "text";
        item["data"] = text;
        item["preview"] = text.left(100).simplified();
    }

    if (!item.isEmpty()) {
        QString newData = item["data"].toString();


        for (int i = 0; i < m_history.size(); ++i) {
            if (m_history.at(i).toMap()["data"].toString() == newData) {
                return;
            }
        }
    
        m_history.prepend(item);

        emit itemAdded(item); 

        if (m_history.size() > 50) m_history.removeLast();
    }
}

void Backend::clearHistory() {
    m_history.clear();  
}






