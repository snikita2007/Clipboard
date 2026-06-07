#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QRegion>
#include <QPainterPath>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;

    window->setColor(Qt::transparent);

    QQmlEngine *engine = qmlEngine(window);
    if (engine) {
        QQmlComponent component(engine);
        component.setData(
            "import QtQuick\n"
            "Rectangle {\n"
            "    anchors.fill: parent\n"
            "    color: palette.windowText.r > 0.5 \n"
            "           ? Qt.rgba(0.11, 0.11, 0.13, 0.68) \n"
            "           : Qt.rgba(0.96, 0.96, 0.98, 0.72)\n"
            "    radius: 14\n"
            "    border.color: palette.windowText.r > 0.5 \n"
            "                  ? Qt.rgba(1, 1, 1, 0.12) \n"
            "                  : Qt.rgba(0, 0, 0, 0.08)\n"
            "    border.width: 1\n"
            "    z: -100\n"
            "}", QUrl());

        QObject *bgObj = component.create();
        QQuickItem *bgItem = qobject_cast<QQuickItem*>(bgObj);
        if (bgItem) {
            bgItem->setParentItem(window->contentItem());
        }
    }

    auto updateBlurRegion = [window]() {
        int radius = 14;
        QSize size = window->size();
        
        QPainterPath path;
        path.addRoundedRect(0, 0, size.width(), size.height(), radius, radius);
        
        QRegion blurRegion(path.toFillPolygon().toPolygon());

        window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", blurRegion);
        window->setProperty("_MUTTER_HINTS", "blur-behind=true");
    };

    QObject::connect(window, &QQuickWindow::widthChanged, window, updateBlurRegion);
    QObject::connect(window, &QQuickWindow::heightChanged, window, updateBlurRegion);

    updateBlurRegion();
    window->setOpacity(0.99);
}