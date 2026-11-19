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
}

void WindowEffects::setUseWindowEffects(bool enabled) {
    m_useWindowEffects = enabled;
}