#import <AppKit/AppKit.h>
#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QPalette>
#include "backend.h"

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}
extern "C" {
    int AddGlassEffectView(void* nativeViewPtr, bool opaque);
    void ConfigureGlassView(int viewId, double cornerRadius, double r, double g, double b, double a);
}
void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;
    window->setColor(Qt::transparent);
    NSView* nativeView = (NSView *)window->winId();
    NSWindow *nativeWindow = [nativeView window];
    [nativeWindow setMovableByWindowBackground:YES];
    int glassId = AddGlassEffectView(nativeView, false);
    if (glassId != -1) {
        ConfigureGlassView(glassId, 30.0, 0.0, 0.0, 0.0, 0.0);
    }
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}