#ifndef WINDOWEFFECTS_H
#define WINDOWEFFECTS_H

#include <QObject>

class WindowEffects : public QObject {
    Q_OBJECT
public:
    explicit WindowEffects(QObject *parent = nullptr);

    Q_INVOKABLE void applyEffects(QObject *window);
};

#endif

