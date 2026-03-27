#import <AppKit/AppKit.h>
#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QPalette>
#include "backend.h"

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    NSView *qtView = (NSView *)window->winId();
    NSWindow *nsWindow = [qtView window];
    if (!nsWindow) return;

    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.appearance = nil;
    nsWindow.movableByWindowBackground = YES;
}

void Backend::checkClipboardNative() {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    long currentCount = [pb changeCount];
    if (currentCount == m_lastChangeCount) {return;}
    m_lastChangeCount = currentCount;
    this->onClipboardChanged(); 
}