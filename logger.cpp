#include "logger.h"

Logger::Logger()
{
}

void Logger::log(Severity lvl, QString msg)
{
    char *lvls[] = { "perf", "trc ", "dbg ", "uerr", "err ", "asrt" };

    qDebug() << lvls[lvl] << msg;
}
