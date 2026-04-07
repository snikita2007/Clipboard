#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QStyleHints>
#include <QColor>
#include <QPalette> 
#include <QRegion>
#include <QPainterPath>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    QPalette palette = QGuiApplication::palette();
    bool isDark = palette.color(QPalette::WindowText).lightness() > palette.color(QPalette::Window).lightness();
    QColor bgColor = isDark ? QColor(32, 32, 32, 250) : QColor(240, 243, 249, 250);
    window->setColor(bgColor);
    
    int radius = 12;
    QSize size = window->size();
    QPainterPath path;
    path.addRoundedRect(0, 0, size.width(), size.height(), radius, radius);
    QRegion mask(path.toFillPolygon().toPolygon());
    window->setMask(mask);

    window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", true);
    window->setOpacity(0.99);
}
