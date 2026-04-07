#ifndef EXAMPLETHREADS_H
#define EXAMPLETHREADS_H

#include <QThread>

class ExampleThreads : public QThread
{
    Q_OBJECT
public:
    explicit ExampleThreads(QString threadName);

    void run();
private:
    QString name;

signals:
    void Data(QStringList ips);

};

#endif

