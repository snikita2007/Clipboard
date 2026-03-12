#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(360, 380);
    
    // Текст Recent:
    main_txtrecent = new QLabel("Recent:");
    main_txtrecent->setAlignment(Qt::AlignLeft);
    
    // Кнопка справа
    btnClear = new QPushButton("Clear");
    btnClear->setFixedSize(80, 25);
    
    // Верхняя строка с Recent и кнопкой
    topLayout = new QHBoxLayout();
    topLayout->addWidget(main_txtrecent);
    topLayout->addStretch();
    topLayout->addWidget(btnClear);
    
    // Текст по центру
    main_txtempty = new QLabel("Clipboard is empty");
    main_txtempty->setAlignment(Qt::AlignCenter);

    //Тестовая кнопка
    testbtn = new QPushButton("Врменная кнопка с большим текстом");
    testbtn->setFixedSize(340, 80);
    
    // Главный layout
    main_layout = new QVBoxLayout(this);
    main_layout->addLayout(topLayout);   
    main_layout->addWidget(testbtn);
    main_layout->addStretch();       
    main_layout->addWidget(main_txtempty, 0, Qt::AlignCenter);  
    main_layout->addStretch();            
}

Widget::~Widget()
{
}