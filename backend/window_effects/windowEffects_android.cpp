#include "WindowEffects.h"
#include <QGuiApplication>
#include <QPalette>
#include <QCoreApplication>
#include <QJniObject>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    Q_UNUSED(rootObject);
    m_window = qobject_cast<QQuickWindow *>(rootObject);
    
    if (m_theme != 0) {
        setTheme(m_theme);
    }
}

void WindowEffects::setTheme(int theme) {
    m_theme = theme;

    QPalette pal = QGuiApplication::palette();
    if (theme == 2) {
        pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
        pal.setColor(QPalette::PlaceholderText, QColor(255, 255, 255, 128));
        QGuiApplication::setPalette(pal);
    } else if (theme == 1) {
        pal.setColor(QPalette::WindowText, QColor(0, 0, 0));
        pal.setColor(QPalette::PlaceholderText, QColor(0, 0, 0, 128));
        QGuiApplication::setPalette(pal);
    } else {
        QGuiApplication::setPalette(QPalette());
    }

    QJniObject activity = QJniObject::queryActivity();
    if (!activity.isValid()) return;

    QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
    if (!window.isValid()) return;

    bool isDark = (theme == 2);
    if (theme == 0) {
        isDark = (QGuiApplication::palette().color(QPalette::WindowText).lightness() > 128);
    }

    QJniObject decorView = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
    if (decorView.isValid()) {
        int flags = decorView.callMethod<jint>("getSystemUiVisibility", "()I");
        
        int SYSTEM_UI_FLAG_LIGHT_STATUS_BAR = 0x00002000;
        int SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR = 0x00000010;

        if (!isDark) {
            flags |= SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            flags |= SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
        } else {
            flags &= ~SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            flags &= ~SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
        }

        decorView.callMethod<void>("setSystemUiVisibility", "(I)V", flags);
    }
}

void WindowEffects::setUseWindowEffects(bool enabled) {
    m_useWindowEffects = enabled;
}