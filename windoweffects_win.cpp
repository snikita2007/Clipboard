#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <QStyleHints>

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd) {
        int backdropType = 2; 
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
        
        BOOL dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? TRUE : FALSE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    }
}

