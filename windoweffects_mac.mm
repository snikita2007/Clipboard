#include "WindowEffects.h"
#include <QQuickWindow>
#import <AppKit/AppKit.h>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    NSView *qtView = (NSView *)window->winId();
    NSWindow *nsWindow = [qtView window];
    if (!nsWindow) return;

    nsWindow.backgroundColor = [NSColor windowBackgroundColor];

    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
    nsWindow.titlebarAppearsTransparent = YES;    

    nsWindow.styleMask |= NSWindowStyleMaskTitled;
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;
    nsWindow.movableByWindowBackground = YES;                 
}