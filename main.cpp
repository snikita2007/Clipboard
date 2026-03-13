#include <QApplication>
#include "widget.h"

int main(int argc, char *argv[]){

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QApplication a(argc, argv);

    Widget main;
    main.show();
     
    return a.exec();
}