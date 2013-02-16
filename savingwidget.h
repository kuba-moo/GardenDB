#ifndef SAVINGWIDGET_H
#define SAVINGWIDGET_H

#include <QWidget>

namespace Ui {
class SavingWidget;
}

class Database;
class SavingThread;

class SavingWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit SavingWidget(Database *db, QWidget *parent = 0);
    ~SavingWidget();
    
signals:
    /* Saving process was completed. */
    void done();
    /* Widget is finished (user clicked exit button). */
    void finished();

private slots:
    void threadDone();
    void advance(int percent, const QString & msg = QString());

private:
    SavingThread *thread;

    Ui::SavingWidget *ui;
};

#endif // SAVINGWIDGET_H
