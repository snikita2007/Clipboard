#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QStyleHints>
#include <windows.h>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (hwnd) {
        static WNDPROC originalWndProc = nullptr;

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style |= (WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_THICKFRAME);
        SetWindowLong(hwnd, GWL_STYLE, style);

        auto customWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT CALLBACK {
            if (msg == WM_NCCALCSIZE && wParam) {
                return 0; 
            }
            if (msg == WM_ERASEBKGND) {
                return 1;
            }
            if (originalWndProc) {
                return CallWindowProc(originalWndProc, hwnd, msg, wParam, lParam);
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        };

        originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(+customWndProc)));

        int backdropType = 2; 
        DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));

        BOOL dark = (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) ? TRUE : FALSE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   

        int cornerPreference = 2; 
        DwmSetWindowAttribute(hwnd, 33, &cornerPreference, sizeof(cornerPreference));

        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}
