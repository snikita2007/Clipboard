#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QPixmap>
#include <QVBoxLayout>
#include <QPushbutton>
#include <QLabel>

class Widget : public QWidget{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    QPushButton *main_first;
    QPushButton *main_second;
    QPushButton *main_third;

    QVBoxLayout *main_layout;
    QPixmap *main_pixmap;

    QLabel *text1;
};

#endif