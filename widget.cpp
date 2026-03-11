#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    main_first = new QPushButton("first");
    main_second = new QPushButton("second");
    main_third = new QPushButton("third");

    text1 = new QLabel("Clipboard:");

    main_layout = new QVBoxLayout(this);

    main_layout->addWidget(text1);
    main_layout->addWidget(main_first);
    main_layout->addWidget(main_second);
    main_layout->addWidget(main_third);
}

Widget::~Widget()
{
}