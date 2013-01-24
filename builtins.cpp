#include "builtins.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

BuiltIns::BuiltIns(QObject *parent) :
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

    reload();
}

void BuiltIns::reload()
{
    foreach (QString category, tableNames.values())
        readTable(category);
}

void BuiltIns::readTable(const QString &name)
{
    QSqlQuery records("SELECT * FROM " + name);

    tables[name] = QLinkedList<QPair<unsigned, QString> >();
    while (records.next()) {
        const unsigned id = records.value(0).toInt();
        const QString value = records.value(1).toString();
        tables[name].append(QPair<unsigned, QString>(id, value));
    }
}

QStringList BuiltIns::getCategories()
{
    return tableNames.keys();
}

unsigned BuiltIns::addValue(const QString &category, const QString &value)
{
    QSqlQuery add(QString("INSERT INTO %1 VALUES(NULL, '%2')")
                    .arg(tableNames[category]).arg(value));

    reload();
    emit changed();

    if (add.lastError().isValid())
            qDebug() << add.lastError().text();

    return add.lastInsertId().toInt();
}

unsigned BuiltIns::countSpecies(const QString &category, const unsigned id)
{
    QSqlQuery count(QString("SELECT Count(*) FROM Species WHERE %1 = %2")
                    .arg(fieldNames[category]).arg(id));

    if (!count.next())
        return 998;

    if (count.lastError().isValid()) {
            qDebug() << count.lastError().text();
            return 999;
    }

    return count.value(0).toInt();
}

void BuiltIns::removeValue(const QString &category, const unsigned id)
{
    QSqlQuery remove(QString("DELETE FROM %1 WHERE id = %2")
                    .arg(tableNames[category]).arg(id));

    reload();
    emit changed();

    if (remove.lastError().isValid())
        qDebug() << remove.lastError().text();
}

void BuiltIns::setValue(const QString &category, const unsigned id, const QString &value)
{
    QSqlQuery update(QString("UPDATE %1 SET name='%2' WHERE id=%3")
                            .arg(tableNames[category]).arg(value).arg(id));

    reload();
    emit changed();

    if (update.lastError().isValid())
        qDebug() << update.lastError().text();
}

const QLinkedList<QPair<unsigned, QString> > &BuiltIns::getValues(const QString &category)
{
    return tables[tableNames[category]];
}
