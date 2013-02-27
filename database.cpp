#include "builtins.h"
#include "database.h"
#include "imagecache.h"
#include "logger.h"
#include "specimenmodel.h"

#include <QApplication>
#include <QElapsedTimer>

const int Database::version = 1;

const unsigned int numCreates = 9;
const QString creates[numCreates] = {
    "CREATE TABLE IF NOT EXISTS Version ("
        "version INTEGER"
    ")",
    "CREATE TABLE IF NOT EXISTS Types ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Flavour ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Flowering ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Frost ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "name TEXT"
    ")",
    "CREATE TABLE IF NOT EXISTS Species ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "no INT,"
        "name TEXT,"
        "grower TEXT,"
        "flowers TEXT,"
        "size TEXT,"
        "desc TEXT,"
        "main_photo INTEGER,"
        "fl_id INTEGER,"
        "fw_id INTEGER,"
        "fr_id INTEGER,"
        "tp_id INTEGER,"
        "FOREIGN KEY(fl_id) REFERENCES Flavour(id),"
        "FOREIGN KEY(fw_id) REFERENCES Flowering(id),"
        "FOREIGN KEY(fr_id) REFERENCES Frost(id),"
        "FOREIGN KEY(tp_id) REFERENCES Types(id)"
    ")",
    "CREATE TABLE IF NOT EXISTS ImagesIndex ("
        "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
        "sp_id INTEGER,"
        "FOREIGN KEY(sp_id) REFERENCES Species(id) ON DELETE CASCADE"
    ")",
    "CREATE TABLE IF NOT EXISTS Images ("
        "id INTEGER PRIMARY KEY,"
        "data BLOB,"
        "FOREIGN KEY(id) REFERENCES ImagesIndex(id) ON DELETE CASCADE"
    ")",
    "CREATE TABLE IF NOT EXISTS ImagesCache ("
        "id INTEGER PRIMARY KEY,"
        "data BLOB,"
        "FOREIGN KEY(id) REFERENCES ImagesIndex(id) ON DELETE CASCADE"
    ")",
};

/* TODO: translate those */
const unsigned int numInserts = 27;
const QString inserts[numInserts] = {
    "INSERT INTO ImagesIndex VALUES(0, 0)",
    "INSERT INTO Types VALUES(0, '')",
    "INSERT INTO Types VALUES(1, 'wielkokwiatowe')",
    "INSERT INTO Types VALUES(2, 'wielokwiatowe')",
    "INSERT INTO Types VALUES(3, '"+QApplication::trUtf8("pnące")+"')",
    "INSERT INTO Types VALUES(4, 'parkowe')",
    "INSERT INTO Types VALUES(5, 'okrywowe')",
    "INSERT INTO Types VALUES(6, 'historyczne')",
    "INSERT INTO Types VALUES(7, 'dzikie')",
    "INSERT INTO Flavour VALUES(0, '')",
    "INSERT INTO Flavour VALUES(1, 'bardzo silny')",
    "INSERT INTO Flavour VALUES(2, 'silny')",
    "INSERT INTO Flavour VALUES(3, '"+QApplication::trUtf8("średni")+"')",
    "INSERT INTO Flavour VALUES(4, '"+QApplication::trUtf8("słaby")+"')",
    "INSERT INTO Flowering VALUES(0, '')",
    "INSERT INTO Flowering VALUES(1, 'maj-czerwiec')",
    "INSERT INTO Flowering VALUES(2, 'czerwiec-lipiec')",
    "INSERT INTO Flowering VALUES(3, 'czerwiec-lipiec powtarza')",
    "INSERT INTO Flowering VALUES(4, 'czerwiec-lipiec "+QApplication::trUtf8("słabo powtarza")+"')",
    "INSERT INTO Frost VALUES(0, '')",
    "INSERT INTO Frost VALUES(1, '5a')",
    "INSERT INTO Frost VALUES(2, '5b')",
    "INSERT INTO Frost VALUES(3, '6a')",
    "INSERT INTO Frost VALUES(4, '6b')",
    "INSERT INTO Frost VALUES(5, '7a')",
    "INSERT INTO Frost VALUES(6, '7b')",
    "INSERT INTO Version VALUES(1)"
};


Database::Database(QString filename, QObject *parent) :
    QObject(parent)
{
    bool newFile = !QFileInfo(filename).exists();

    if (!QSqlDatabase::database().isValid())
        database = QSqlDatabase::addDatabase("QSQLITE");
    else
        database = QSqlDatabase::database();
    database.setDatabaseName(filename);
    database.open();

    if (database.isOpenError()) {
        Log(UserError) << trUtf8("Unable to open database:")
                       << database.lastError().text();
        return;
    }

    /* Init database - if new. */
    if (newFile && !init()) {
        Log(UserError) << trUtf8("Unable to initialize new database");
        database.close();
        return;
    }

    /* Handle DB versions. */
    int dbVersion = getVersion();
    if (dbVersion < 0) {
        Log(UserError) << trUtf8("Unable to open database: unknown version");
        database.close();
        return;
    }
    if (dbVersion > version) {
        Log(UserError) << trUtf8("Unable to open database: it was created"
                                 " with a newer version of the program");
        database.close();
        return;
    }

    if (dbVersion < version && !upgrade(database, dbVersion)) {
        Log(UserError) << trUtf8("Unable to open database: it was created"
                                 " with an older version of the program"
                                 " and upgrade had failed");
        database.close();
        return;
    }

    if (!heal()) {
        Log(UserError) << trUtf8("Unable to open database: fixing data"
                                 " format has failed");
        database.close();
        return;
    }

    bi = new Builtins(this);
    if (!bi->load()) {
        Log(UserError) << trUtf8("Unable to read builtin values from database");
        database.close();
        return;
    }

    ic = new ImageCache(this);
    if (!ic->load()) {
        database.close();
        return;
    }

    sm = new SpecimenModel(this);
    if (!sm->load(ic)) {
        Log(UserError) << trUtf8("Unable to read species from database");
        database.close();
        return;
    }
}

Database::~Database()
{
    if (database.isOpen())
        database.close();
}

bool Database::isModified()
{
    if (bi->isModified() || ic->isModified() || sm->isModified())
        return true;

    return false;
}

bool Database::init()
{
    QElapsedTimer t; t.start();
    Log(Debug) << "Database init";

    if (!database.transaction()) {
        database.close();
        Log(Error) << "start transaction" << database.lastError().text();
        return false;
    }

    QSqlQuery result;
    for (unsigned i=0; i < numCreates && !result.lastError().isValid(); i++)
        result = database.exec(creates[i]);

    for (unsigned i=0; i < numInserts && !result.lastError().isValid(); i++)
        result = database.exec(inserts[i]);

    if (result.lastError().isValid()) {
        Log(Error) << "init" << result.lastQuery() << " | "
                   << result.lastError().text();
        return false;
    }

    if (!database.commit()) {
        database.close();
        Log(Error) << "commit transaction" << database.lastError().text();
        return false;
    }

    result.finish();
    Log(Performance) << "Database init took" << t.elapsed() << "ms";

    return true;
}

int Database::getVersion()
{
    QSqlQuery check("SELECT name FROM sqlite_master WHERE type='table' AND name='Version'");
    if (check.lastError().isValid()) {
        Log(Error) <<  "getVersion check:" << check.lastError().text();
        return -1;
    }

    if (!check.next()) {
        return 0;
    }

    check.exec("SELECT * FROM Version");
    if (check.lastError().isValid()) {
        Log(Error) << "getVersion get version:" << check.lastError().text();
        return -1;
    }
    if (!check.next()) {
        Log(Error) << "getVersion Version empty";
        return -1;
    }

    return check.record().value("version").toInt();
}

#include "image.h" /* For cache size. */

bool Database::upgrade(QSqlDatabase &db, const int from)
{
    Log(Debug) << "upgrade database from" << from << "to" << version;
    Log(UserInfo) << trUtf8("Database format needs to be "
                            "upgraded please press ok and wait");

    if (from != 0)
        return false;

    QElapsedTimer t; t.start();
    /* We can leave it hanging on failure - close will rollback. */
    if (!db.transaction()) {
        Log(Error) << "upgrade can't begin";
        return false;
    }

    /* Species */
    QSqlQuery update("ALTER TABLE Species ADD no INT");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Species addNo:" << update.lastError().text();
        return false;
    }

    update.exec("ALTER TABLE Species ADD grower TEXT");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Species addGrower:" << update.lastError().text();
        return false;
    }

    /* Images */
    update.exec("ALTER TABLE Images RENAME TO Images_v0");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Images mv:" << update.lastError().text();
        return false;
    }

    update.exec("ALTER TABLE ImagesCache RENAME TO ImagesCache_v0");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 ImagesCache mv:" << update.lastError().text();
        return false;
    }

    update.exec("CREATE TABLE ImagesIndex ("
                "id INTEGER PRIMARY KEY ASC AUTOINCREMENT,"
                "sp_id INTEGER,"
                "FOREIGN KEY(sp_id) REFERENCES Species(id) ON DELETE CASCADE"
            ")");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 ImagesIndex create:" << update.lastError().text();
        return false;
    }

    update.exec("CREATE TABLE Images ("
                "id INTEGER PRIMARY KEY,"
                "data BLOB,"
                "FOREIGN KEY(id) REFERENCES ImagesIndex(id) ON DELETE CASCADE"
            ")");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Images create:" << update.lastError().text();
        return false;
    }

    update.exec("CREATE TABLE ImagesCache ("
                "id INTEGER PRIMARY KEY,"
                "data BLOB,"
                "FOREIGN KEY(id) REFERENCES ImagesIndex(id) ON DELETE CASCADE"
            ")");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 ImagesCache create:" << update.lastError().text();
        return false;
    }

    update.exec("SELECT * FROM Images_v0");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Images read:" << update.lastError().text();
        return false;
    }
    while (update.next()) {
        int id = update.record().value("id").toInt();
        int sp_id = update.record().value("sp_id").toInt();
        QByteArray data = update.record().value("data").toByteArray();

        /* Skip first '0' and '1' - '0' will be reinserted later. */
        if (!id || id == 1)
            continue;

        QSqlQuery insert(QString("INSERT INTO ImagesIndex VALUES(%1, %2)")
                         .arg(id).arg(sp_id));
        if (insert.lastError().isValid()) {
            Log(Error) << "upgrade v0 ImagesIndex insert:"
                       << insert.lastError().text();
            return false;
        }

        QPixmap pixmap;
        pixmap.loadFromData(data);

        QByteArray array;
        QBuffer buf(&array);
        buf.open(QIODevice::WriteOnly);
        pixmap.save(&buf, "JPG", Image::Quality);

        insert.prepare("INSERT INTO Images VALUES(:id, :image)");
        insert.bindValue(":id", id);
        insert.bindValue(":image", array);
        insert.exec();
        if (insert.lastError().isValid()) {
            Log(Error) << "upgrade v0 Images insert:" << insert.lastError().text();
            return false;
        }

        QPixmap cache = pixmap.scaled(Image::CachedImageSize,
                                      Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation);

        array.clear();
        QBuffer buf2(&array);
        buf2.open(QIODevice::WriteOnly);
        cache.save(&buf2, "JPG", Image::CacheQuality);

        insert.prepare("INSERT INTO ImagesCache VALUES(:id, :image)");
        insert.bindValue(":id", id);
        insert.bindValue(":image", array);
        insert.exec();
        if (insert.lastError().isValid()) {
            Log(Error) << "upgrade v0 Images insert:" << insert.lastError().text();
            return false;
        }
    }

    update.exec("UPDATE Species SET main_photo=0 WHERE main_photo=1");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Species not 1:" << update.lastError().text();
        return false;
    }

    update.exec("DROP TABLE Images_v0");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Images rm:" << update.lastError().text();
        return false;
    }

    update.exec("DROP TABLE ImagesCache_v0");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 ImagesCache rm:" << update.lastError().text();
        return false;
    }

    /* Insert NULL-values. */
    update.exec("INSERT INTO Types VALUES(0, '')");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Types add zero:" << update.lastError().text();
        return false;
    }

    update.exec("INSERT INTO Flavour VALUES(0, '')");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Flavour add zero:" << update.lastError().text();
        return false;
    }

    update.exec("INSERT INTO Flowering VALUES(0, '')");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Flowering add zero:" << update.lastError().text();
        return false;
    }

    update.exec("INSERT INTO Frost VALUES(0, '')");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Frost add zero:" << update.lastError().text();
        return false;
    }

    update.exec("INSERT INTO ImagesIndex VALUES(0, 0)");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 ImagesIndex add zero:" << update.lastError().text();
        return false;
    }

    /* Create version table. */
    update.exec("CREATE TABLE Version (version INTEGER)");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Version create:" << update.lastError().text();
        return false;
    }

    update.exec("INSERT INTO Version VALUES(1)");
    if (update.lastError().isValid()) {
        Log(Error) << "upgrade v0 Version add:" << update.lastError().text();
        return false;
    }

    if (!db.commit()) {
        Log(Error) << "upgrade commit";
        return false;
    }

    Log(Performance) << "DB upgrade took" << t.elapsed() << "ms";
    return true;
}

bool Database::heal()
{
    QSqlQuery query("UPDATE Species SET main_photo=0 "
                    "WHERE main_photo NOT IN (SELECT id FROM ImagesIndex)");
    if (query.lastError().isValid()) {
        Log(Error) << "heal species" << query.lastError().text();
        return false;
    }
    if (query.numRowsAffected())
        Log(Warning) << "heal species affected" << query.numRowsAffected();

    /* TODO: heal other stuff as well */

    return true;
}
