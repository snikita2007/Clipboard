#import <AppKit/AppKit.h>
#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QPalette>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    NSView *qtView = (NSView *)window->winId();
    NSWindow *nsWindow = [qtView window];
    if (!nsWindow) return;

    nsWindow.titleVisibility = NSWindowTitleHidden;
    // nsWindow.backgroundColor = [NSColor windowBackgroundColor];
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.appearance = nil;
    nsWindow.movableByWindowBackground = YES;
}
