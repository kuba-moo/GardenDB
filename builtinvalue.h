#ifndef BUILTINVALUE_H
#define BUILTINVALUE_H

#include <QObject>

class QSqlDatabase;

class BuiltinValue : public QObject
{
    Q_OBJECT
public:
    explicit BuiltinValue(const QString &table, const int id,
                          const QString &value, bool isNew, QObject *parent = 0);
    
    int id() const { return _id; }
    const QString &value() const { return _value; }

    void setValue(const QString &value);

    bool isModified() { return state != Unmodified; }
    bool remove();
    bool save(QSqlDatabase &db);

    operator QString() const { return _value; }
    operator int() const { return _id; }

signals:
    
public slots:

private:
    enum State {
        Unmodified,
        New,
        Modified,
        Removed
    } state;

    int _id;
    QString originalValue, _value;
    QString _table;
};

#endif // BUILTINVALUE_H
