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
    
    /* Returns all categories of built-in values. */
    const QStringList &getCategories();

    /* Get values from sepcified category. */
    const QLinkedList<QPair<unsigned, QString> > &getValues(const QString &category);

    /* Adds new value to given category, return id of added value or zero
     * on error. Change is propagated by emitting signal changed.
     */
    unsigned addValue(const QString &category, QString value = trUtf8("New value"));

signals:
    void changed();

private:
    /* Reads table of given name into tables. Does not emit changed. */
    void readTable(const QString &name);

    QStringList categories;
    QHash<QString, QLinkedList<QPair<unsigned, QString> > > tables;
};

#endif // BUILTINS_H
