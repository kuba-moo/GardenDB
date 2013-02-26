#include "builtins.h"
#include "database.h"
#include "imagecache.h"
#include "maintable.h"
#include "ui_maintable.h"
#include "specimen.h"
#include "specimenmodel.h"
#include "specimenrenderer.h"

#include <QScrollBar>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QStyledItemDelegate>
#include <QMessageBox>

MainTable::MainTable(Database *db, const int scrollTo, const int selected, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTable)
{
    ui->setupUi(this);

    database = db;

    ui->listView->setModel(db->specimenModel());
    ui->listView->setItemDelegate(new SpecimenRenderer(db));

    ui->nSpecies->setText(QString::number(ui->listView->model()->rowCount()));

    QModelIndex index = db->specimenModel()->index(selected, 0);
    if (index.isValid()) {
        QItemSelectionModel *selection = new QItemSelectionModel(db->specimenModel());
        selection->select(index, QItemSelectionModel::Select);
        selection->setCurrentIndex(index, QItemSelectionModel::Select);
        ui->listView->setSelectionModel(selection);
    }
    index = db->specimenModel()->index(scrollTo, 0);
    if (index.isValid())
        ui->listView->scrollTo(index);

    connect(ui->add, SIGNAL(clicked()), SLOT(addRow()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeRow()));
    connect(db->imageCache(), SIGNAL(changed()), ui->listView, SLOT(update()));
    connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(doubleClicked(QModelIndex)));
}

MainTable::~MainTable()
{
    delete ui;
}

int MainTable::selectedRow()
{
    const QModelIndexList &selected = ui->listView->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return -1;
    return selected.first().row();
}

int MainTable::scrollRow()
{
    return ui->listView->verticalScrollBar()->value();
}

void MainTable::addRow()
{
    SpecimenModel *sm = database->specimenModel();
    sm->insertRow(sm->rowCount());

    emit requestEditor(sm->index(sm->rowCount() - 1, 0));
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

    ui->nSpecies->setText(QString::number(ui->listView->model()->rowCount()));
}

void MainTable::doubleClicked(const QModelIndex &index)
{
    QPoint mouse = ui->listView->mapFromGlobal(cursor().pos());

    if (mouse.x() < 100) {
        Specimen *s = (Specimen *)index.data().toULongLong();
        if (s && s->getMainPhotoId()) {
            emit requestGallery(index);
            return;
        }
    }

    emit requestEditor(index);
}
