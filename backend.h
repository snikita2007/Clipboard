#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
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

public slots:
    void onClipboardChanged();

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();
    QVariantList history() const { return m_history; }
    Q_INVOKABLE void copyToClip(QVariant data);

private:
    QVariantList m_history;
    QString m_cacheDir;
};

#endif