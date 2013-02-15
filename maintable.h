#ifndef MAINTABLE_H
#define MAINTABLE_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
    class MainTable;
}

class BuiltIns;
class Database;
class ImageCache;

class MainTable : public QWidget
{
    Q_OBJECT

signals:
    void rowDetails(QModelIndex);

public:
    explicit MainTable(Database *db, QWidget *parent = 0);
    ~MainTable();

private slots:
    /* Add new row to model and scroll to it. */
    void addRow();
    /* Ask if user is sure and remove row from model. */
    void removeRow();

private:
    Database *database;
    Ui::MainTable *ui;
};

#endif // MAINTABLE_H
