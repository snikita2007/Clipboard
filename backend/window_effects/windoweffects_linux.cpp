#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QStyleHints>
#include <QColor>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    bool isDark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
    QColor bgColor = isDark ? QColor(32, 32, 32, 250) : QColor(240, 243, 249, 250);
    window->setColor(bgColor);

    window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", true);
    
    window->setOpacity(0.98);
}