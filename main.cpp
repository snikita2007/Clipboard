#include <QApplication>
#include "widget.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    Widget main;
    main.show();
     
    return a.exec();
}