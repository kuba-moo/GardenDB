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
