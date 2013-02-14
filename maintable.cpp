#include "builtins.h"
#include "database.h"
#include "imagecache.h"
#include "maintable.h"
#include "ui_maintable.h"
#include "imagerenderer.h"

#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QStyledItemDelegate>
#include <QMessageBox>

MainTable::MainTable(Database *db, BuiltIns *builtIns, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTable)
{
    ui->setupUi(this);

    connect(ui->add, SIGNAL(clicked()), SIGNAL(addRow()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeRow()));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)),
            SIGNAL(rowDetails(QModelIndex)));
    connect(builtIns, SIGNAL(changed()), SLOT(loadData()));
    connect(db->imageCache(), SIGNAL(changed()), SLOT(loadData()));

    loadData();

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->horizontalHeader()->moveSection(5, 1);
    ui->tableView->horizontalHeader()->moveSection(9, 3);
    ui->tableView->setItemDelegate(new QSqlRelationalDelegate(ui->tableView));
    ui->tableView->setItemDelegateForColumn(5, new ImageRenderer(db->imageCache(),
                                                                 ui->tableView));
    ui->tableView->setColumnWidth(5, 83);
    ui->tableView->verticalHeader()->setDefaultSectionSize(64);
}

MainTable::~MainTable()
{
    delete ui;
}

void MainTable::loadData()
{
    qDebug() << "Main table reload";

    /* 0     1       2    3    4          5     6     7     8     9*/
    /*id  name flowers size desc main_photo fl_id fw_id fr_id tp_id*/
    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
    model->setTable("Species");
    model->setHeaderData(1, Qt::Horizontal, trUtf8("Name"));
    model->setHeaderData(2, Qt::Horizontal, trUtf8("Flowers"));
    model->setHeaderData(3, Qt::Horizontal, trUtf8("Bush size"));
    model->setHeaderData(4, Qt::Horizontal, trUtf8("Description"));
    model->setHeaderData(5, Qt::Horizontal, trUtf8("Photo"));
    model->setHeaderData(6, Qt::Horizontal, trUtf8("Flavour"));
    model->setRelation(6, QSqlRelation("Flavour", "id", "name"));
    model->setHeaderData(7, Qt::Horizontal, trUtf8("Flowering"));
    model->setRelation(7, QSqlRelation("Flowering", "id", "name"));
    model->setHeaderData(8, Qt::Horizontal, trUtf8("Frost resistance"));
    model->setRelation(8, QSqlRelation("Frost", "id", "name"));
    model->setHeaderData(9, Qt::Horizontal, trUtf8("Type"));
    model->setRelation(9, QSqlRelation("Types", "id", "name"));
    model->select();

    QAbstractItemModel *oldModel =  ui->tableView->model();
    ui->tableView->setModel(model);
    delete oldModel;
}

void MainTable::removeRow()
{
    int ret = QMessageBox::question(this, trUtf8("Remove row"),
                                    trUtf8("Are you sure you want to remove\n"
                                           "this row?\n"
                                           "This change cannot be undone."),
                                    QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::Yes && ui->tableView->model()
            && ui->tableView->selectionModel()->hasSelection())
    {
        int row = ui->tableView->selectionModel()->currentIndex().row();
        ui->tableView->model()->removeRow(row);
    }
}
