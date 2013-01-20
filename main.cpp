#include <QtGui/QApplication>
#include "omain.h"
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load(":/translations/pl");
    a.installTranslator(&qtTranslator);

    OMain w;
    w.show();

    if (argc != 1)
        w.doOpen(argv[1]);

    return a.exec();
}
