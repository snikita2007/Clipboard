#include "WindowEffects.h"
#include <QQuickWindow>
#include <QMetaObject>
#import <AppKit/AppKit.h>

#ifndef NSApplicationDidChangeEffectiveAppearanceNotification
#define NSApplicationDidChangeEffectiveAppearanceNotification @"NSApplicationDidChangeEffectiveAppearanceNotification"
#endif


void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    NSView *qtView = reinterpret_cast<NSView *>(window->winId());
    if (!qtView) return;

    NSWindow *nsWindow = qtView.window;
    if (!nsWindow) return;

    nsWindow.opaque = NO;
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.titlebarAppearsTransparent = YES;

    nsWindow.titleVisibility = NSWindowTitleHidden;
    nsWindow.styleMask |= NSWindowStyleMaskFullSizeContentView;

    NSView *oldContentView = nsWindow.contentView;
    if (!oldContentView) return;

    NSUserInterfaceItemIdentifier containerId = @"ClipboardEffectsContainer";
    NSUserInterfaceItemIdentifier effectId = @"ClipboardVibrancyBackground";

    NSView *containerView = nil;
    if ([oldContentView.identifier isEqualToString:containerId]) {
        containerView = oldContentView;
    } else {
        containerView = [[NSView alloc] initWithFrame:oldContentView.bounds];
        containerView.identifier = containerId;
        containerView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

        if (qtView.superview == oldContentView) {
            [qtView removeFromSuperviewWithoutNeedingDisplay];
        }

        nsWindow.contentView = containerView;
    }

    for (NSView *sub in containerView.subviews) {
        if ([sub.identifier isEqualToString:effectId]) {
            [sub removeFromSuperview];
            break;
        }
    }

    NSVisualEffectView *effectView = [[NSVisualEffectView alloc] initWithFrame:containerView.bounds];
    effectView.identifier = effectId;
    effectView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    // More "native" macOS window look
    effectView.material = NSVisualEffectMaterialWindowBackground;

    // We place the effect view inside the window, so this should be WithinWindow
    effectView.blendingMode = NSVisualEffectBlendingModeWithinWindow;
    effectView.state = NSVisualEffectStateActive;

    [containerView addSubview:effectView positioned:NSWindowBelow relativeTo:nil];

    if (qtView.superview != containerView) {
        [qtView removeFromSuperviewWithoutNeedingDisplay];
        qtView.frame = containerView.bounds;
        qtView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        [containerView addSubview:qtView positioned:NSWindowAbove relativeTo:effectView];
    }
}
