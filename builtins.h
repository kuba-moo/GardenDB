#ifndef BUILTINS_H
#define BUILTINS_H

#include "builtinvalue.h"

#include <QHash>
#include <QLinkedList>
#include <QObject>
#include <QPair>
#include <QStringList>

class QSqlDatabase;

class Builtins : public QObject
{
    Q_OBJECT
public:
    explicit Builtins(QObject *parent = 0);
        
    /* Load information from database, return true on success. */
    bool load();

    bool isModified();
    int countModified();
    bool save(QSqlDatabase &db);

    /* Returns descriptive names of all categories of built-in values. */
    QStringList getCategories();

    /* Get values from sepcified category. */
    const QLinkedList<BuiltinValue *> &getValues(const QString &category);
    const QLinkedList<BuiltinValue *> &getValuesTr(const QString &localCategory);

    QString getValue(const QString &category, const int id);
    /* Adds new value to given category, return id of added value or zero
     * on error. Change is propagated by emitting signal changed. */
    unsigned addValueTr(const QString &localCategory, const QString &value = trUtf8("New value"));
    /* Remove value and all species which are pointing to it. */
    void removeValueTr(const QString &localCategory, const int id);
    /* Set item to given value. */
    void setValueTr(const QString &localCategory, const int id, const QString &newValue);

signals:
    void changed();
    void oneSaved();

private:
    /* Reads table of given name into tables, return true on success. */
    bool readTable(const QString &name);

    QHash<QString, QString> tableNames, fieldNames;
    QHash<QString, QLinkedList<BuiltinValue *> > tables;
    QHash<QString, int> lastInsertId;

    QList<BuiltinValue *> removed;
};

#endif // BUILTINS_H
