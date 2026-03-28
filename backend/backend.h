#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QClipboard>
#include <QImage>
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)

signals:
    void historyChanged();
    void itemAdded(QVariant item);

public slots:
    void onClipboardChanged();

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();
    QVariantList history() const { return m_history; }
    Q_INVOKABLE void copyToClip(QVariant data);
    void checkClipboardNative();

private:
    QVariantList m_history;
    QString m_cacheDir;
    #ifdef Q_OS_MAC
    long m_lastChangeCount = -1; 
    #endif
};

#endif