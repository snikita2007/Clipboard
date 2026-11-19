#include "WindowEffects.h"
#include <QQuickWindow>
#include <QGuiApplication>
#include <QRegion>
#include <QPainterPath>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QPalette>
#include <QColor>

WindowEffects::WindowEffects(QObject *parent) : QObject(parent) {}

void WindowEffects::applyEffects(QObject *rootObject) {
    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (!window) return;
    m_window = window;

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

    auto updateBlurRegion = [window, this]() {
        if (!m_useWindowEffects) {
            window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", QRegion());
            window->setProperty("_MUTTER_HINTS", "blur-behind=false");
            return;
        }
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
    window->setOpacity(m_useWindowEffects ? 0.99 : 1.0);
}

void WindowEffects::setTheme(int theme) {
    m_theme = theme;

    QPalette pal = QGuiApplication::palette();
    if (theme == 2) {
        pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
        pal.setColor(QPalette::PlaceholderText, QColor(255, 255, 255, 128));
        QGuiApplication::setPalette(pal);
    } else if (theme == 1) {
        pal.setColor(QPalette::WindowText, QColor(0, 0, 0));
        pal.setColor(QPalette::PlaceholderText, QColor(0, 0, 0, 128));
        QGuiApplication::setPalette(pal);
    } else {
        QGuiApplication::setPalette(QPalette());
    }
}

void WindowEffects::setUseWindowEffects(bool enabled) {
    m_useWindowEffects = enabled;
    if (!m_window) return;
    
    if (enabled) {
        m_window->setOpacity(0.99);
        int radius = 14;
        QSize size = m_window->size();
        QPainterPath path;
        path.addRoundedRect(0, 0, size.width(), size.height(), radius, radius);
        QRegion blurRegion(path.toFillPolygon().toPolygon());
        m_window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", blurRegion);
        m_window->setProperty("_MUTTER_HINTS", "blur-behind=true");
    } else {
        m_window->setOpacity(1.0);
        m_window->setProperty("_KDE_NET_WM_BLUR_BEHIND_REGION", QRegion());
        m_window->setProperty("_MUTTER_HINTS", "blur-behind=false");
    }
}