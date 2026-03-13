#import <AppKit/AppKit.h>

extern "C" void applyMacVibrancy(unsigned long long winId) {
    NSView *qtView = (NSView *)winId;
    NSWindow *window = [qtView window];

    window.titlebarAppearsTransparent = YES;
    window.titleVisibility = NSWindowTitleHidden;
    window.styleMask |= NSWindowStyleMaskFullSizeContentView;

    [window setBackgroundColor:[NSColor colorWithDeviceRed:0.15 green:0.15 blue:0.15 alpha:1.0]];
}
