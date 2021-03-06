/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "database.h"
#include "builtins.h"
#include "imagecache.h"
#include "logger.h"
#include "savingthread.h"
#include "specimenmodel.h"

#include <QSqlDatabase>

SavingThread::SavingThread(Database *database, QObject *parent) :
    QThread(parent)
{
    db = database;
    alreadySaved = 0;
    dbFilename = QSqlDatabase::database().databaseName();
    _hasErrors = true;
}

void SavingThread::oneDone()
{
    alreadySaved++;
    int percent = 100*alreadySaved/totalToBeSaved;

    emit advance(percent, msg);
}

void SavingThread::run()
{
    Log(Debug) << "Saving started tid:" << (unsigned long long)currentThreadId();

    QSqlDatabase database;
    if (!QSqlDatabase::database("saver").isValid())
        database = QSqlDatabase::addDatabase("QSQLITE", "saver");
    else
        database = QSqlDatabase::database("saver");

    database.setDatabaseName(dbFilename);
    database.open();

    if (database.isOpenError()) {
        emit error(trUtf8("Unable to open database for saving"));
        return;
    }

    Builtins *bi = db->builtins();
    ImageCache *ic = db->imageCache();
    SpecimenModel *sm = db->specimenModel();

    totalToBeSaved = bi->countModified() + ic->countModified() + sm->countModified();

    if (!database.transaction()) {
        database.close();
        emit error(trUtf8("Unable to open database for saving, try again"));
        Log(Error) << "start transaction" << database.lastError().text();
        return;
    }

    connect(bi, SIGNAL(oneSaved()), SLOT(oneDone()));
    msg = trUtf8("Saving built in values...");
    if (!bi->save(database)) {
        database.close();
        emit error(trUtf8("Unable to save changes to builtin values"));
        return;
    }

    connect(ic, SIGNAL(oneSaved()), SLOT(oneDone()));
    msg = trUtf8("Saving images...");
    if (!ic->save(database)) {
        database.close();
        emit error(trUtf8("Unable to save changes to images"));
        return;
    }

    connect(sm, SIGNAL(oneSaved()), SLOT(oneDone()));
    msg = trUtf8("Saving species...");
    if (!sm->save(database)) {
        database.close();
        emit error(trUtf8("Unable to save changes to species"));
        return;
    }

    if (!database.commit()) {
        database.close();
        emit error(trUtf8("Unable to commit changes to database, your changes were lost"));
        Log(Error) << "commit transaction" << database.lastError().text();
        return;
    }

    _hasErrors = false;
    database.close();
}
