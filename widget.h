#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QPixmap>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QIcon>

class Widget : public QWidget{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
private:
    QLabel *main_txtempty;//Текст пустого буфера
    QLabel *main_txtrecent;//Текст недавние
    QPushButton *btnClear;//Отчитска
    QPushButton *btnSetting;
    QPushButton *testbtn;

    QVBoxLayout *main_layout;
    QHBoxLayout *topLayout;
    QPixmap *main_pixmap;

};

#endif