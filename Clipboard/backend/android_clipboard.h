#pragma once

#include <QString>
#include <QStringList>

class Backend;

// Android only: background sync service and reading the clipboard from the background.
// Java side: android/src/com/clipboard/app/
namespace AndroidClipboard {

// Connects the Java side to the backend and starts the background service
void start(Backend *backend);
void stop();

// Call before the app changes the clipboard itself, so this change is not sent back
void markOwnClipboardWrite();
void readClipboardFiles();
void setClipboardFiles(const QStringList &paths);

// "auto", "no-logs", "no-overlay" or "manual"
QString backgroundStatus();

}
