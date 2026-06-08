#import <AppKit/AppKit.h>
#include <QQuickWindow>
#include <QPalette>
#include <QColor>

#include "WindowEffects.h"

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

extern "C" {
    int AddGlassEffectView(void* nativeViewPtr, bool opaque);
    void ConfigureGlassView(int viewId, double cornerRadius, double r, double g, double b, double a);
}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;
    m_window = window;
    window->setColor(Qt::transparent);
    
    NSView* nativeView = (NSView *)window->winId();
    NSWindow *nativeWindow = [nativeView window];
    
    int glassId = AddGlassEffectView(nativeView, !m_useWindowEffects);
    if (glassId != -1) {
        ConfigureGlassView(glassId, 28.0, 0.0, 0.0, 0.0, 0.0);
    }

    nativeView.wantsLayer = YES;
    nativeView.layer.cornerRadius = 28.0;
    nativeView.layer.masksToBounds = YES;

    if (m_theme == 2) {
        nativeWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
    } else if (m_theme == 1) {
        nativeWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
    }
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
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
    NSView* nativeView = (NSView *)m_window->winId();
    NSWindow *nativeWindow = [nativeView window];
    if (theme == 2) {
        nativeWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
    } else if (theme == 1) {
        nativeWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
    } else {
        nativeWindow.appearance = nil;
    }
}

void WindowEffects::setUseWindowEffects(bool enabled) {
    m_useWindowEffects = enabled;
    if (!m_window) return;
    NSView* nativeView = (NSView *)m_window->winId();
    
    int glassId = AddGlassEffectView(nativeView, !enabled);
    if (glassId != -1) {
        ConfigureGlassView(glassId, 28.0, 0.0, 0.0, 0.0, 0.0);
    }

    nativeView.wantsLayer = YES;
    nativeView.layer.cornerRadius = 28.0;
    nativeView.layer.masksToBounds = YES;
}