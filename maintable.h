#ifndef MAINTABLE_H
#define MAINTABLE_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
    class MainTable;
}

class ImageCache;
class BuiltIns;

class MainTable : public QWidget
{
    Q_OBJECT

signals:
    void addRow();
    void rowDetails(QModelIndex);

public:
    explicit MainTable(ImageCache *imageCache, BuiltIns *builtIns, QWidget *parent = 0);
    ~MainTable();

public slots:
    /* Called after row has been added. */
    void loadData();

private slots:
    /* Ask if user is sure and remove row from model. */
    void removeRow();

private:
    Ui::MainTable *ui;
};

#endif // MAINTABLE_H
