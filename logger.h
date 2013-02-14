#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

enum Severity {
    Performance,
    Trace,
    Debug,
    UserError,
    Error,
    Assert
};

class Logger
{
public:
    Logger();

    static void log(Severity lvl, QString msg);
};

#endif // LOGGER_H
