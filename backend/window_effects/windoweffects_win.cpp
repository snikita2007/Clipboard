#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#include <QStyleHints>
#include <windows.h>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {
}

class Win32RepaintFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        if (eventType != "windows_generic_MSG") return false;
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_ERASEBKGND) {
            *result = 1; 
            return true; 
        }
        if (msg->message == WM_NCCALCSIZE) {
            *result = 0;
            return true;
        }
        return false;
    }
};

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd) {
        static Win32RepaintFilter filter;
        static bool filterInstalled = false;
        if (!filterInstalled) {
            QCoreApplication::instance()->installNativeEventFilter(&filter);
            filterInstalled = true;
        }

        int backdropType = 3; 
        DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));

        BOOL dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? TRUE : FALSE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   

        int cornerPreference = 2; 
        DwmSetWindowAttribute(hwnd, 33, &cornerPreference, sizeof(cornerPreference));

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= (WS_CAPTION);
        SetWindowLong(hwnd, GWL_STYLE, style);

        window->showMinimized();
        window->showNormal();
    }
}