#include "clipboard_files.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>

bool ClipboardFiles::isImage(const QString &path)
{
    static const QStringList extensions = {"png", "jpg", "jpeg", "gif", "bmp", "ico", "webp"};
    return extensions.contains(QFileInfo(path).suffix().toLower());
}

QString ClipboardFiles::safeName(QString name)
{
    name.replace('\\', '/');
    name = name.section('/', -1);
    name.replace(QRegularExpression(QStringLiteral("[\\x00-\\x1f<>:\"/\\\\|?*]")), "_");
    while (name.endsWith('.') || name.endsWith(' ')) name.chop(1);
    // Keep below common filesystem limits, including UTF-8 names on Android.
    while (name.toUtf8().size() > 240) name.chop(1);
    if (name.isEmpty() || name == "." || name == "..") name = "attachment";
    static const QRegularExpression reserved("^(CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])($|\\.)",
                                             QRegularExpression::CaseInsensitiveOption);
    if (reserved.match(name).hasMatch()) name.prepend('_');
    return name;
}

QString ClipboardFiles::store(const QString &cacheDir, const QString &name, const QByteArray &bytes)
{
    // Content-specific directories preserve distinct files with the same name.
    const QString digest = QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
    const QString directory = cacheDir + "/received_" + digest;
    if (!QDir().mkpath(directory)) return {};
    const QString path = directory + '/' + safeName(name);
    QFile existing(path);
    if (existing.open(QIODevice::ReadOnly) && existing.read(bytes.size() + 1) == bytes) return path;
    existing.close();
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) return {};
    return path;
}
