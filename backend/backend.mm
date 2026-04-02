#include "backend.h"
#import <AppKit/AppKit.h>

void Backend::checkClipboardNative() {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    long currentCount = [pb changeCount];
    if (currentCount == m_lastChangeCount) {return;}
    m_lastChangeCount = currentCount;
    this->onClipboardChanged(); 
}