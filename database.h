#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>

class Builtins;
class ImageCache;
class SpecimenModel;

/**
 * @brief The Database File class, UI - business logic proxy
 *
 * This class takes care of file loading and unloading procedures including
 * initialization of empty databases, upgrade of old databases to a new format
 * and saving loaded state.
 */

class Database : public QObject
{
    Q_OBJECT
public:
    /* Load/initialize database in given file. */
    explicit Database(QString filename, QObject *parent = 0);
    ~Database();

    /* True if database was opened and loaded successfully. */
    bool isOk() { return database.isOpen(); }

    bool isModified();

    /* Aggregate accessors. */
    Builtins *builtins() const { return bi; }
    ImageCache *imageCache() const { return ic; }
    SpecimenModel *specimenModel() const { return sm; }

signals:

public slots:

private:
    /* Current database format version.*/
    static const int version;

    /* Figure out and return format used in the database loaded. */
    int getVersion();

    /* Try to initialize new database, return true on success. */
    bool init();
    /* Change format of database to current, return true on success. */
    bool upgrade(QSqlDatabase &db, const int from);
    /* Find and fix errors in database. */
    bool heal();

    QSqlDatabase database;
    Builtins *bi;
    ImageCache *ic;
    SpecimenModel *sm;
};

#endif // DATABASE_H
