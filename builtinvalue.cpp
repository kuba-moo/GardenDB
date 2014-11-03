/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "builtinvalue.h"
#include "logger.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

BuiltinValue::BuiltinValue(const QString &table, const int id, const QString &value, bool isNew, QObject *parent) :
    QObject(parent)
{
    state = isNew ? New : Unmodified;

    _id = id;
    originalValue = _value = value;
    _table = table;
}

void BuiltinValue::setValue(const QString &value)
{
    _value = value;

    if (state == Modified || state == Unmodified)
        state = (_value == originalValue) ? Unmodified : Modified;
}

bool BuiltinValue::remove()
{
    if (state == New)
        return false;

    state = Removed;
    return true;
}

#include "unistd.h"

bool BuiltinValue::save(QSqlDatabase &db)
{
    QSqlQuery query(db);

    if (state == New) {
        query.exec(QString("INSERT INTO %1 VALUES(%2,'%3')")
                   .arg(_table).arg(_id).arg(_value));
    } else if (state == Modified) {
        query.exec(QString("UPDATE %1 SET name='%2' WHERE id=%3")
                   .arg(_table).arg(_value).arg(_id));
    } else if (state == Removed) {
        query.exec("DELETE FROM " + _table + " WHERE id=" + QString::number(_id));
    }

    if (query.lastError().isValid()) {
        Log(Error) << query.lastError().text() << " | " << query.lastQuery();
        return false;
    }
    if (query.numRowsAffected() != 1)
        Log(Warning) << "Builtin save Rows affected"
                     << query.numRowsAffected() << "state" << state;

    if (state != Removed)
        state = Unmodified;
    return true;
}
