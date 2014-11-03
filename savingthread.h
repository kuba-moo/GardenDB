/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#ifndef SAVINGTHREAD_H
#define SAVINGTHREAD_H

#include <QThread>

class Database;

class SavingThread : public QThread
{
    Q_OBJECT
public:
    explicit SavingThread(Database *database, QObject *parent = 0);
    
    void run();

    bool hasErrors() const { return _hasErrors; }

signals:
    void advance(int);
    void advance(int, const QString &);
    void error(const QString &);

private slots:
    void oneDone();

private:
    Database *db;
    int totalToBeSaved, alreadySaved;
    QString msg, dbFilename;
    /* Saving process failed. */
    bool _hasErrors;
};

#endif // SAVINGTHREAD_H
