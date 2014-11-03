/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
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
    Log &operator <<(const qint64 &number);

private:
    Severity level;
    QStringList list;
};

#endif // LOGGER_H
