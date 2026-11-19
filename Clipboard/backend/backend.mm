#include "backend.h"
#import <AppKit/AppKit.h>

void Backend::checkClipboardNative() {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    long currentCount = [pb changeCount];
    if (currentCount == m_lastChangeCount) {return;}
    m_lastChangeCount = currentCount;
    if (m_firstCount == 0)
        this->onClipboardChanged(); 
    m_firstCount = 0;
}