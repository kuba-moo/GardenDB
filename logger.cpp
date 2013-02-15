#include "logger.h"

QString lvls[] = { "perf", "trc ", "dbg ", "uinf", "warn", "uerr", "err ", "asrt" };

Logger::Logger()
{
}

void Logger::log(Severity lvl, QString msg)
{
    qDebug() << QString(lvls[lvl] + " " + msg);
}

Log::Log(Severity lvl)
{
    level = lvl;
}

Log::~Log()
{
    Logger::log(level, list);
}

Log &Log::operator <<(const QString &string)
{
    list.append(" ").append(string);
    return *this;
}

Log &Log::operator <<(const int &number)
{
    list.append(" ").append(QString::number(number));
    return *this;
}
