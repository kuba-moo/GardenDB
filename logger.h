#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>
#include <QMutex>
#include <QWidget>

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
    static Logger *instance() { if (!_i) _i = new Logger(); return _i; }

    void log(Severity lvl, QString msg);

    void setWindow(QWidget *w) { window = w; }

private:
    Logger();
    Logger(const Logger &) {}
    Logger &operator=(const Logger &) { return *this; }
    ~Logger();

    QMutex mutex;

    QFile *file;
    QWidget *window;
    static Logger *_i;
};

class Log
{
public:
    Log(Severity lvl);
    ~Log();

    Severity getLevel() const { return level; }

    Log &operator <<(const QString &string);
    Log &operator <<(const QStringList &list);
    Log &operator <<(const int &number);
    Log &operator <<(const unsigned long long &number);

private:
    Severity level;
    QStringList list;
};

#endif // LOGGER_H
