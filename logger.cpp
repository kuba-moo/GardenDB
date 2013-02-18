#include "logger.h"

#include <QApplication>
#include <QMessageBox>

QString lvls[] = { "perf", "trc ", "dbg ", "uinf", "warn", "uerr", "err ", "asrt" };

Logger *Logger::_i = 0;

Logger::Logger()
{
    file = new QFile(QDir::homePath() + "/.garden.log");

    if (!file->open(QIODevice::Append))
        qDebug() << "Opening logfile failed";
}

Logger::~Logger()
{
    file->flush();
    delete file;
}

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

    msg = lvls[lvl] + " " + msg;
    qDebug() << msg;
    if (file->isOpen()) {
        msg += "\n";
        file->write(msg.toAscii());
        file->flush();
    }
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
