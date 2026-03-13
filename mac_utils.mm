#import <AppKit/AppKit.h>

extern "C" void applyMacVibrancy(unsigned long long winId) {
    NSView *qtView = (NSView *)winId;
    NSWindow *window = [qtView window];
    
    window.titlebarAppearsTransparent = YES;
    [window setBackgroundColor:[NSColor colorWithDeviceRed:0.12 green:0.12 blue:0.12 alpha:1.0]];
}