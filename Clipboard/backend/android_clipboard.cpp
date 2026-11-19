#include "android_clipboard.h"
#include "backend.h"

#include <QCoreApplication>
#include <QtCore/qcoreapplication_platform.h>
#include <QJniEnvironment>
#include <QJniObject>
#include <QPointer>
#include <QDebug>

namespace {

constexpr char BRIDGE_CLASS[] = "com/clipboard/app/ClipboardBridge";

QPointer<Backend> g_backend;

// Called by Java (any thread) with text copied on the phone
void onClipboardText(JNIEnv *, jclass, jstring jtext)
{
    if (!jtext) return;
    const QString text = QJniObject(jtext).toString();

    QMetaObject::invokeMethod(qApp, [text]() {
        if (g_backend) g_backend->processExternalText(text);
    }, Qt::QueuedConnection);
}

void onClipboardFiles(JNIEnv *env, jclass, jobjectArray jpaths)
{
    if (!jpaths) return;
    QStringList paths;
    for (jsize i = 0; i < env->GetArrayLength(jpaths); ++i) {
        jobject path = env->GetObjectArrayElement(jpaths, i);
        paths.append(QJniObject(path).toString());
        env->DeleteLocalRef(path);
    }
    QMetaObject::invokeMethod(qApp, [paths]() {
        if (g_backend) g_backend->processExternalFiles(paths);
    }, Qt::QueuedConnection);
}

QJniObject appContext()
{
    return QNativeInterface::QAndroidApplication::context();
}

}

void AndroidClipboard::start(Backend *backend)
{
    g_backend = backend;

    const JNINativeMethod methods[] = {
        {"nativeOnClipboardText", "(Ljava/lang/String;)V", reinterpret_cast<void *>(onClipboardText)},
        {"nativeOnClipboardFiles", "([Ljava/lang/String;)V", reinterpret_cast<void *>(onClipboardFiles)},
    };
    QJniEnvironment env;
    if (!env.registerNativeMethods(BRIDGE_CLASS, methods, 2)) {
        qWarning() << "AndroidClipboard: cannot register native methods";
        return;
    }

    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "start", "(Landroid/content/Context;)V",
                                       appContext().object());

    // Text shared to the app while it was not running
    const QString pending = QJniObject::callStaticObjectMethod(
        BRIDGE_CLASS, "takePendingText", "()Ljava/lang/String;").toString();
    if (!pending.isEmpty()) {
        QMetaObject::invokeMethod(qApp, [pending]() {
            if (g_backend) g_backend->processExternalText(pending);
        }, Qt::QueuedConnection);
    }
    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "deliverPendingFiles", "()V");
}

void AndroidClipboard::stop()
{
    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "stop", "(Landroid/content/Context;)V",
                                       appContext().object());
    g_backend = nullptr;
}

void AndroidClipboard::markOwnClipboardWrite()
{
    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "markOwnClipboardWrite", "()V");
}

void AndroidClipboard::readClipboardFiles()
{
    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "readClipboardFiles", "(Landroid/content/Context;)V",
                                       appContext().object());
}

void AndroidClipboard::setClipboardFiles(const QStringList &paths)
{
    QJniEnvironment env;
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(jsize(paths.size()), stringClass, nullptr);
    env->DeleteLocalRef(stringClass);
    for (qsizetype i = 0; i < paths.size(); ++i) {
        const QJniObject path = QJniObject::fromString(paths[i]);
        env->SetObjectArrayElement(array, jsize(i), path.object());
    }
    QJniObject::callStaticMethod<void>(BRIDGE_CLASS, "setClipboardFiles",
                                       "(Landroid/content/Context;[Ljava/lang/String;)V", appContext().object(), array);
    env->DeleteLocalRef(array);
}

QString AndroidClipboard::backgroundStatus()
{
    return QJniObject::callStaticObjectMethod(BRIDGE_CLASS, "backgroundStatus",
                                              "(Landroid/content/Context;)Ljava/lang/String;",
                                              appContext().object()).toString();
}
