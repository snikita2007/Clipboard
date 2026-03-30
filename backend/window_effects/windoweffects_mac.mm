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
    void* nativeView = (void*)window->winId();
    int glassId = AddGlassEffectView(nativeView, false);
    if (glassId != -1) {
        ConfigureGlassView(glassId, 30.0, 0.0, 0.0, 0.0, 0.0);
        SetGlassViewAdaptiveAppearance(glassId, 2);
        SetGlassViewVariant(glassId, 16);
    }
}