#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(360, 380);
    QString buttonStyle;
    
    //Стиль для виндоус
    #ifdef Q_OS_WIN
    setAttribute(Qt::WA_TranslucentBackground); 
    HWND hwnd = (HWND)this->winId();
    DWORD mica = 2;
    DwmSetWindowAttribute(hwnd, 38, &mica, sizeof(mica));
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    this->setStyleSheet("background: transparent;");

    buttonStyle = 
        "QPushButton {"
        "   background-color: rgba(200, 200, 200, 0.1);" 
        "   border: none;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 0.2);"
        "}";

    //Стиль для мак
    #elif defined(Q_OS_MAC)
        applyMacVibrancy((unsigned long long)this->winId());
        
        buttonStyle = "QPushButton { background-color: #3d3d3d; color: white; border-radius: 10px; }"
        "QPushButton:hover {"
        "   background-color: rgba(200, 200, 200, 0.2);"
        "}";
    #else
    buttonStyle = "QPushButton { padding: 5px; }"; //заглушка для остальных ос
    #endif

    // Текст Recent:
    main_txtrecent = new QLabel("Recent");
    main_txtrecent->setAlignment(Qt::AlignLeft);
    
    // Кнопка справа
    btnClear = new QPushButton("Clear all");
    btnClear->setFixedSize(80, 25);
    btnClear->setStyleSheet(buttonStyle);

    //Кнопка настроек
    btnSetting = new QPushButton();
    btnSetting->setIcon(QIcon(":/assets/settings.png"));
    btnSetting->setFixedSize(25, 25);
    btnSetting->setIconSize(QSize(20, 20));
    btnSetting->setStyleSheet(buttonStyle);
    
    // Верхняя строка с Recent и кнопкой
    topLayout = new QHBoxLayout();
    topLayout->setAlignment(Qt::AlignVCenter); 
    topLayout->addWidget(main_txtrecent, 0, Qt::AlignVCenter);
    topLayout->addStretch();
    topLayout->addWidget(btnClear, 0, Qt::AlignVCenter);
    topLayout->addWidget(btnSetting, 0, Qt::AlignVCenter);
    
    // Текст по центру
    main_txtempty = new QLabel("Clipboard is empty");
    main_txtempty->setAlignment(Qt::AlignCenter);

    //Тестовая кнопка
    testbtn = new QPushButton("Врменная кнопка с большим текстом");
    testbtn->setFixedSize(340, 80);
    testbtn->setStyleSheet(buttonStyle);
    
    // Главный layout
    main_layout = new QVBoxLayout(this);

    main_layout->setContentsMargins(10, 5, 10, 10);
    main_layout->setSpacing(15);

    main_layout->addLayout(topLayout);   
    main_layout->addWidget(testbtn, 0, Qt::AlignCenter);
    main_layout->addStretch();       
    main_layout->addWidget(main_txtempty, 0, Qt::AlignCenter);  
    main_layout->addStretch();

}

Widget::~Widget()
{
}