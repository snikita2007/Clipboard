#include "examplethreads.h"
#include <QNetworkInterface>
#include <QThread>
#include <ip.h>
#include "examplethreads.h"
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun> 


ExampleThreads::ExampleThreads(QString threadName) :
    name(threadName) { }

void ExampleThreads::run()
{
    QVector<QFuture<void>> futures;
    NetworkScanner *scanner = new NetworkScanner();

    for (int arg = 1; arg < 51; arg++) {  
        QFuture<void> future = QtConcurrent::run([scanner, arg]() {
            scanner->scanNetwork(arg * 5);
        });
        futures.append(future); 
    }

    for (auto &future : futures) {
        future.waitForFinished();
    }
}

