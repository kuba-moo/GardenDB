#include "builtins.h"
#include "database.h"
#include "imagecache.h"
#include "maintable.h"
#include "ui_maintable.h"
#include "specimenmodel.h"
#include "specimenrenderer.h"

#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QStyledItemDelegate>
#include <QMessageBox>

MainTable::MainTable(Database *db, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTable)
{
    ui->setupUi(this);

    database = db;

    ui->listView->setModel(db->specimenModel());
    ui->listView->setItemDelegate(new SpecimenRenderer(db));

    connect(ui->add, SIGNAL(clicked()), SLOT(addRow()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeRow()));
//  connect(builtIns, SIGNAL(changed()), ui->listView, SLOT(update()));
    connect(db->imageCache(), SIGNAL(changed()), ui->listView, SLOT(update()));
    connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)),
            SIGNAL(rowDetails(QModelIndex)));;
}

MainTable::~MainTable()
{
    delete ui;
}

void MainTable::addRow()
{
    SpecimenModel *sm = database->specimenModel();
    sm->insertRow(sm->rowCount());

    QModelIndex index = sm->index(sm->rowCount(QModelIndex()) - 1, 0);
    QItemSelectionModel *selection = new QItemSelectionModel(sm);
    selection->select(index, QItemSelectionModel::Select);
    selection->setCurrentIndex(index, QItemSelectionModel::Select);
    ui->listView->setSelectionModel(selection);
    ui->listView->scrollTo(index);
}

void MainTable::removeRow()
{
    const QListView * const view = ui->listView;
    if (!view->model() || !view->selectionModel()->hasSelection()) {
        QMessageBox::information(this, trUtf8("Remove row"),
                                 trUtf8("Choose row which you want to remove"));
        return;
    }

    int ret = QMessageBox::question(this, trUtf8("Remove row"),
                                    trUtf8("Sure you want to remove this row?"),
                                    QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        int row = view->selectionModel()->currentIndex().row();
        view->model()->removeRow(row);
    }
}
