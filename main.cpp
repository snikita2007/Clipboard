#include <QApplication>
#include <QWidget>
#include <QDebug>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QWidget w;
    w.show();
    qDebug() << "Окно\n";
    return a.exec();
}