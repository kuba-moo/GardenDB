/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include <QtGui/QApplication>
#include "logger.h"
#include "omain.h"
#include <QTranslator>

class Application : public QApplication
{
public:
    Application(int &argc, char **argv) : QApplication(argc, argv) {}
    virtual ~Application() {}

    virtual bool notify(QObject *receiver, QEvent *event)
    {
        timer.start();
        bool ret = QApplication::notify(receiver, event);

        if (event && receiver && timer.elapsed() > 100) {
            if (event->type() != 52)
                Log(Warning) << "Processing event =" << (int)event->type()
                             << "receiver=" << receiver->objectName()
                             << "took" << (int)timer.elapsed() << "ms";
            else
                Log(Warning) << "Processing event =" << (int)event->type()
                             << "took" << (int)timer.elapsed() << "ms";
        }

        return ret;
    }

private:
    QElapsedTimer timer;
};

int main(int argc, char *argv[])
{
    Application a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load(":/translations/pl");
    a.installTranslator(&qtTranslator);

    OMain w;
    w.show();

    if (argc != 1)
        w.doOpen(argv[1]);

    return a.exec();
}
