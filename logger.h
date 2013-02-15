#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

enum Severity {
    Performance,
    Trace,
    Debug,
    UserInfo,
    Warning,
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

class Log
{
public:
    Log(Severity lvl);
    ~Log();

    Log &operator <<(const QString &string);
    Log &operator <<(const int &number);

private:
    Severity level;
    QString list;
};

#endif // LOGGER_H
