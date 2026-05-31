#include "WindowEffects.h"
#include <dwmapi.h>
#include <QQuickWindow>
#include <windows.h>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *win = qobject_cast<QQuickWindow *>(rootObject);
    if (!win) return;
    HWND hwnd = reinterpret_cast<HWND>(win->winId());

    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(GetStockObject(NULL_BRUSH)));
    SetWindowLong(hwnd, GWL_STYLE, (GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME | WS_CAPTION) & ~(WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX));

    int bdt = 2; DwmSetWindowAttribute(hwnd, 38, &bdt, sizeof(bdt));

    static WNDPROC origProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(+[](HWND h, UINT m, WPARAM wp, LPARAM lp) -> LRESULT CALLBACK {
        if (m == WM_ERASEBKGND) return 1; 
        if (m == WM_NCCALCSIZE && wp) {
            RECT* r = &reinterpret_cast<NCCALCSIZE_PARAMS*>(lp)->rgrc[0];
            r->left += 8; r->right -= 8; r->bottom -= 8;
            return 0;
        }
        return CallWindowProc(origProc, h, m, wp, lp);
    })));

    ShowWindow(hwnd, SW_MINIMIZE);
}