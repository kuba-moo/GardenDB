#ifndef BUILTINS_H
#define BUILTINS_H

#include <QHash>
#include <QLinkedList>
#include <QObject>
#include <QPair>
#include <QStringList>

class BuiltIns : public QObject
{
    Q_OBJECT
public:
    explicit BuiltIns(QObject *parent = 0);
    
    /* Returns descriptive names of all categories of built-in values. */
    QStringList getCategories();

    /* Get values from sepcified category. */
    const QLinkedList<QPair<unsigned, QString> > &getValues(const QString &category);

    QString getValueTr(const QString &engCategory, const unsigned id);
    /* Adds new value to given category, return id of added value or zero
     * on error. Change is propagated by emitting signal changed. */
    unsigned addValue(const QString &category, const QString &value = trUtf8("New value"));
    /* Return number of species which have given value. */
    unsigned countSpecies(const QString &category, const unsigned id);
    /* Remove value and all species which are pointing to it. */
    void removeValue(const QString &category, const unsigned id);
    /* Set item to given value. */
    void setValue(const QString &category, const unsigned id, const QString &value);

signals:
    void changed();

private:
    /* Reload values from database. */
    void reload();
    /* Reads table of given name into tables. Does not emit changed. */
    void readTable(const QString &name);

    QHash<QString, QString> tableNames, fieldNames;
    QHash<QString, QLinkedList<QPair<unsigned, QString> > > tables;
};

#endif // BUILTINS_H
