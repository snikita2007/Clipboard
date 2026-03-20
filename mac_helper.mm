#include <AppKit/AppKit.h>
#include <QWindow>

void enableMacVibrancy(QWindow *window) {
    NSView *view = (NSView *)window->winId();
    NSWindow *nswin = [view window];
    
    // Создаем слой размытия
    NSVisualEffectView *vibrantView = [[NSVisualEffectView alloc] initWithFrame:view.bounds];
    vibrantView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    vibrantView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
    vibrantView.state = NSVisualEffectStateActive;

    [view addSubview:vibrantView positioned:NSWindowBelow relativeTo:nil];
}
