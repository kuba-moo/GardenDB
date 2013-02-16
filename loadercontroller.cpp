#include "imageloader.h"
#include "loadercontroller.h"
#include "logger.h"

#include <QCoreApplication>
#include <QMetaType>

Q_DECLARE_METATYPE(QVector<int>)

LoaderController::LoaderController(QObject *parent) :
    QThread(parent)
{
    idle = false;
    start();
    qRegisterMetaType<QVector<int> >("QVector<int>");


}

void LoaderController::run()
{
    loader = new ImageLoader();
    connect(loader, SIGNAL(idle()), this, SLOT(loaderIdle()));
    connect(loader, SIGNAL(doneCached(int,QImage*)),
            SIGNAL(doneCached(int,QImage*)));
    connect(loader, SIGNAL(doneFullsize(int,QImage*)),
            SIGNAL(doneFullsize(int,QImage*)));
    loader->start();

    exec();

    idle = false;
    delete loader;
    loader = 0;
}

void LoaderController::getCached(QList<int> l)
{
    caches.append(l);
    tryToPostWork();
}

void LoaderController::getFullsize(int id)
{
    fulls.push_back(id);
    tryToPostWork();
}

void LoaderController::tryToPostWork()
{
    if (!idle)
        return;

    if (!fulls.empty()) {
        QMetaObject::invokeMethod(loader, "getFullsize", Q_ARG(int, fulls.takeFirst()));
        idle = false;
    } else if (!caches.empty()) {
        QVector<int> v;
        for (int i = 0; i < 8 && !caches.empty(); i++)
            v.append(caches.takeFirst());
        QMetaObject::invokeMethod(loader, "getCached", Q_ARG(QVector<int>, v));
        idle = false;
    }
}

void LoaderController::loaderIdle()
{
    idle = true;
    tryToPostWork();
}
