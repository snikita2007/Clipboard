#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QDebug>
#include <QGuiApplication>

class Backend : public QObject {
    Q_OBJECT
public:
    explicit Backend(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void doWork();

    // Q_INVOKABLE void addToClip();

    // Q_INVOKABLE void removeFromClip();

    Q_INVOKABLE void clearClip();

};

#endif