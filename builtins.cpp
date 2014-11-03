/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "builtins.h"
#include "logger.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Builtins::Builtins(QObject *parent) :
    QObject(parent)
{
    /* Without this tmp thing keys don't get translated... */
    QStringList tmp;
    tmp << trUtf8("Types") << trUtf8("Flavour")
        << trUtf8("Frost resistance") << trUtf8("Flowering time");
    tableNames[trUtf8("Types")] = "Types";
    tableNames[trUtf8("Flavour")] = "Flavour";
    tableNames[trUtf8("Frost resistance")] = "Frost";
    tableNames[trUtf8("Flowering time")] = "Flowering";
    fieldNames[trUtf8("Types")] = "tp_id";
    fieldNames[trUtf8("Flavour")] = "fl_id";
    fieldNames[trUtf8("Frost resistance")] = "fr_id";
    fieldNames[trUtf8("Flowering time")] = "fw_id";
}

bool Builtins::load()
{
    foreach (QString category, tableNames.values())
        if (!readTable(category))
            return false;

    return true;
}

bool Builtins::isModified()
{
    if (removed.size())
        return true;

    foreach (QString category, tableNames.values()) {
        const QLinkedList<BuiltinValue *> &table = tables[category];

        QLinkedList<BuiltinValue *>::const_iterator i;
        for (i = table.begin(); i != table.end(); i++)
            if ((*i)->isModified())
                return true;
    }

    return false;
}

int Builtins::countModified()
{
    int modified = removed.size();

    foreach (QString category, tableNames.values()) {
        const QLinkedList<BuiltinValue *> &table = tables[category];

        QLinkedList<BuiltinValue *>::const_iterator i;
        for (i = table.begin(); i != table.end(); i++)
            if ((*i)->isModified())
                modified++;
    }

    return modified;
}

bool Builtins::save(QSqlDatabase &db)
{
    while (removed.size()) {
        BuiltinValue *biv = removed.takeFirst();
        if (!biv->save(db))
            return false;
        emit oneSaved();
        delete biv;
    }

    foreach (QString category, tableNames.values()) {
        const QLinkedList<BuiltinValue *> &table = tables[category];

        QLinkedList<BuiltinValue *>::const_iterator i;
        for (i = table.begin(); i != table.end(); i++)
            if ((*i)->isModified()) {
                if (!(*i)->save(db))
                    return false;

                emit oneSaved();
            }
    }

    return true;
}

bool Builtins::readTable(const QString &name)
{
    QSqlQuery records("SELECT * FROM " + name);
    if (records.lastError().isValid()) {
        Log(Error) << "Builtins unable to read table" << name
                   << ":" << records.lastError().text();
        return false;
    }

    tables[name] = QLinkedList<BuiltinValue *>();
    while (records.next()) {
        const int id = records.value(0).toInt();
        const QString value = records.value(1).toString();
        if (id)
            tables[name].append(new BuiltinValue(name, id, value, false, this));
    }

    /* Can't do usual "find max" thing here, because species may point to items
     * after last existing, if last items where removed. */
    QSqlQuery seq("SELECT seq FROM sqlite_sequence WHERE name='" + name + "'");
    if (seq.lastError().isValid()) {
        Log(Error) << "Builtins unable to read sequence for table" << name
                   << ":" << seq.lastError().text();
        return false;
    }
    while (seq.next())
        lastInsertId[name] = seq.value(0).toInt();

    return true;
}

QStringList Builtins::getCategories()
{
    return tableNames.keys();
}

QString Builtins::getValue(const QString &category, const int id)
{
    const QLinkedList<BuiltinValue *> &table = tables[category];

    QLinkedList<BuiltinValue *>::const_iterator i;
    for (i = table.begin(); i != table.end(); i++)
        if (**i == id)
            return **i;

    return QString();
}

unsigned Builtins::addValueTr(const QString &localCategory, const QString &value)
{
    const QString &category = tableNames[localCategory];
    QLinkedList<BuiltinValue *> &table = tables[category];

    QLinkedList<BuiltinValue *>::const_iterator i;
    for (i = table.constBegin(); i != table.constEnd(); i++)
        if (value == **i) {
            Log(UserError) << trUtf8("Duplicated value");
            return 0;
        }

    tables[category].append(new BuiltinValue(category, ++lastInsertId[category],
                                             value, true, this));
    emit changed();
    return lastInsertId[category];
}

void Builtins::removeValueTr(const QString &localCategory, const int id)
{
    const QString &category = tableNames[localCategory];
    QLinkedList<BuiltinValue *> &table = tables[category];


    QLinkedList<BuiltinValue *>::iterator i;
    for (i = table.begin(); i != table.end(); i++)
        if (id == **i) {
            if ((*i)->remove())
                removed.append(*i);

            tables[category].removeOne(*i);

            emit changed();
            break;
        }
}

void Builtins::setValueTr(const QString &localCategory, const int id, const QString &newValue)
{
    const QString &category = tableNames[localCategory];
    QLinkedList<BuiltinValue *> &table = tables[category];

    QLinkedList<BuiltinValue *>::iterator i;
    for (i = table.begin(); i != table.end(); i++)
        if (newValue == **i) {
            Log(UserError) << trUtf8("Duplicated value");
            return;
        }

    for (i = table.begin(); i != table.end(); i++)
        if (id == **i) {
            (*i)->setValue(newValue);
            break;
        }

    emit changed();
}

const QLinkedList<BuiltinValue *> &Builtins::getValues(const QString &category)
{
    return tables[category];
}

const QLinkedList<BuiltinValue *> &Builtins::getValuesTr(const QString &localCategory)
{
    return getValues(tableNames[localCategory]);
}
