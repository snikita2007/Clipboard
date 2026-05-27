#import <AppKit/AppKit.h>
#include <QQuickWindow>

#include "WindowEffects.h"

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
    int glassId = AddGlassEffectView(nativeView, false);
    if (glassId != -1) {
        ConfigureGlassView(glassId, 30.0, 0.0, 0.0, 0.0, 0.0);
    }
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}