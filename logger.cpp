#include "logger.h"

#include <QApplication>
#include <QMessageBox>

QString lvls[] = { "perf", "trc ", "dbg ", "uinf", "warn", "uerr", "err ", "asrt" };

Logger *Logger::_i = 0;

void Logger::log(Severity lvl, QString msg)
{
    if (lvl == UserInfo && window) {
        QMessageBox::information(window, QApplication::trUtf8("Information"), msg);
        return;
    }

    if (lvl == UserError && window) {
        QMessageBox::critical(window, QApplication::trUtf8("Error"), msg);
        return;
    }

    if (!window && (lvl == UserInfo || lvl == UserError))
        Log(Assert) << "Logger: window not set";

    qDebug() << QString(lvls[lvl] + " " + msg);
}

Log::Log(Severity lvl)
{
    level = lvl;
}

Log::~Log()
{
    Logger::instance()->log(level, list.join(" "));
}

Log &Log::operator <<(const QString &string)
{
    list << string;
    return *this;
}

Log &Log::operator <<(const QStringList &l)
{
    list.append(l);
    return *this;
}

Log &Log::operator <<(const int &number)
{
    list << QString::number(number);
    return *this;
}

Log &Log::operator <<(const unsigned long long &number)
{
    list << QString::number(number);
    return *this;
}
