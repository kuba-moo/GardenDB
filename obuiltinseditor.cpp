#include "obuiltinseditor.h"
#include "ui_obuiltinseditor.h"

#include <QSqlTableModel>
#include <QDebug>

OBuiltInsEditor::OBuiltInsEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OBuiltInsEditor)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->listWidget,SIGNAL(currentRowChanged(int)),SLOT(changeTable(int)));
    connect(ui->add, SIGNAL(clicked()), SLOT(addRow()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeRow()));
}

OBuiltInsEditor::~OBuiltInsEditor()
{
    delete ui;
}

void OBuiltInsEditor::changeTable(int row_id)
{
    static QString tables[] = { trUtf8("Types"), trUtf8("Flavour"),
                                trUtf8("Flowering"), trUtf8("Frost") };
    if (ui->tableView->model())
        delete ui->tableView->model();
    QSqlTableModel *model = new QSqlTableModel(ui->tableView);

    model->setTable(tables[row_id]);
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    model->setHeaderData(1, Qt::Horizontal, trUtf8("Name"));

    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true);
}

void OBuiltInsEditor::addRow()
{
    QSqlTableModel *model = (QSqlTableModel*)ui->tableView->model();
    if (model)
    {
        int lastRow = model->rowCount();
        /* Careful with column no, model sees them all */
        model->insertRows(lastRow, 1);
        ui->tableView->edit(model->index(lastRow, 1));
    }
}

void OBuiltInsEditor::removeRow()
{
    if (ui->tableView->model()
            && ui->tableView->selectionModel()->hasSelection())
        ui->tableView->model()->removeRow(
                    ui->tableView->selectionModel()->currentIndex().row());
}
