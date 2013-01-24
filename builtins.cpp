#include "builtins.h"

#include <QSqlQuery>
#include <QVariant>

BuiltIns::BuiltIns(QObject *parent) :
    QObject(parent)
{
    categories << "Types" << "Flavour" << "Frost" << "Flowering";
    foreach (QString category, categories)
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

const QStringList &BuiltIns::getCategories()
{
    return categories;
}

unsigned BuiltIns::addValue(const QString &category, QString value)
{
    value = category;

    emit changed();

    return 0;
}

const QLinkedList<QPair<unsigned, QString> > &BuiltIns::getValues(const QString &category)
{
    return tables[category];
}
