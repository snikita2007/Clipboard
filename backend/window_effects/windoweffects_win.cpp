#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <QStyleHints>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd) {
        int backdropType = 2; 
        DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));

        BOOL dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? TRUE : FALSE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   

        window->showMinimized();
        window->showNormal();
    }
}


