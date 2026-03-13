#import <AppKit/AppKit.h>
#include <QWidget>

extern "C" void applyMacVibrancy(unsigned long long winId) {
    NSView *view = (NSView *)winId;
    NSWindow *window = [view window];
    NSVisualEffectView *vibrantView = [[NSVisualEffectView alloc] initWithFrame:[[window contentView] bounds]];
    [vibrantView setMaterial:NSVisualEffectMaterialUnderWindowBackground];
    [vibrantView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
    [vibrantView setState:NSVisualEffectStateActive];
    [vibrantView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [[window contentView] addSubview:vibrantView positioned:NSWindowBelow relativeTo:nil];
    [window setTitlebarAppearsTransparent:YES];
}