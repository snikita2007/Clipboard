#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <windows.h>
#include <QGuiApplication>
#include <QPalette>
#include <QColor>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *win = qobject_cast<QQuickWindow *>(rootObject);
    if (!win) return;
    m_window = win;
    HWND hwnd = reinterpret_cast<HWND>(win->winId());

    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(GetStockObject(NULL_BRUSH)));
    SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME | WS_CAPTION) & ~(WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));

    int bdt = m_useWindowEffects ? 3 : 2;
    DwmSetWindowAttribute(hwnd, 38, &bdt, sizeof(bdt));

    static WNDPROC origProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(+[](HWND h, UINT m, WPARAM wp, LPARAM lp) -> LRESULT CALLBACK {
        if (m == WM_ERASEBKGND) return 1; 
        if (m == WM_NCCALCSIZE && wp) {
            RECT* r = &reinterpret_cast<NCCALCSIZE_PARAMS*>(lp)->rgrc[0];
            r->left += 8; r->right -= 8; r->bottom -= 8;
            return 0;
        }
        return CallWindowProc(origProc, h, m, wp, lp);
    })));

    BOOL dark = (m_theme == 2);
    if (m_theme == 0) {
        dark = (QGuiApplication::palette().color(QPalette::WindowText).lightness() > 128);
    }
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));

    ShowWindow(hwnd, SW_MINIMIZE);
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

    if (!m_window) return;
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    BOOL dark = FALSE;
    if (theme == 2) {
        dark = TRUE;
    } else if (theme == 0) {
        dark = (QGuiApplication::palette().color(QPalette::WindowText).lightness() > 128);
    }
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
}

void WindowEffects::setUseWindowEffects(bool enabled) {
    m_useWindowEffects = enabled;
    if (!m_window) return;
    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    int bdt = enabled ? 3 : 2;
    DwmSetWindowAttribute(hwnd, 38, &bdt, sizeof(bdt));
}