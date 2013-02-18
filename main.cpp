#include <QtGui/QApplication>
#include "logger.h"
#include "omain.h"
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* This cruft is probably not needed actually. */
    Log(Debug) << "Run from" << QDir::current().absolutePath();
    Log(Debug) << "Libpath was" << QCoreApplication::libraryPaths();

    Log(Debug) << "Libpath now is" << a.libraryPaths();

    QLibrary sqlib("./sqldrivers/qsqlite4.dll");
    sqlib.load();
    QLibrary sqlib2("qsqlited4.dll");
    sqlib2.load();

    Log(Debug) << "my library loaded"<<sqlib.isLoaded() <<sqlib2.isLoaded();


    QTranslator qtTranslator;
    qtTranslator.load(":/translations/pl");
    a.installTranslator(&qtTranslator);

    OMain w;
    w.show();

    if (argc != 1)
        w.doOpen(argv[1]);

    return a.exec();
}
