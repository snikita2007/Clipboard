#pragma once

#include <QByteArray>
#include <QString>

namespace ClipboardFiles {
bool isImage(const QString &path);
QString safeName(QString name);
QString store(const QString &cacheDir, const QString &name, const QByteArray &bytes);
}
