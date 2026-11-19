#ifndef WINDOWEFFECTS_H
#define WINDOWEFFECTS_H

#include <QObject>
#include <QQuickWindow>

class WindowEffects : public QObject {
    Q_OBJECT
public:
    explicit WindowEffects(QObject *parent = nullptr);

    Q_INVOKABLE void applyEffects(QObject *window);
    Q_INVOKABLE void setTheme(int theme);
    Q_INVOKABLE void setUseWindowEffects(bool enabled);

private:
    int m_theme = 0;
    bool m_useWindowEffects = true;
    QQuickWindow *m_window = nullptr;
};

#endif