#include "builtins.h"
#include "obuiltinseditor.h"
#include "ui_obuiltinseditor.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlTableModel>

OBuiltInsEditor::OBuiltInsEditor(BuiltIns *builtIns, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OBuiltInsEditor)
{
    ui->setupUi(this);

    builtins = builtIns;
    lastRowId = 0;
    blockEdits = true;

    connect(ui->categories,SIGNAL(currentRowChanged(int)),SLOT(setTable(int)));
    connect(ui->add, SIGNAL(clicked()), SLOT(addRow()));
    connect(ui->remove, SIGNAL(clicked()), SLOT(removeRow()));
    connect(builtins, SIGNAL(changed()), SLOT(reload()));
    connect(ui->values, SIGNAL(itemChanged(QListWidgetItem*)),
            SLOT(edit(QListWidgetItem*)));
    connect(ui->close, SIGNAL(clicked()), SIGNAL(finished()));

    reload();
}

OBuiltInsEditor::~OBuiltInsEditor()
{
    delete ui;
}

void OBuiltInsEditor::reload()
{
    ui->categories->clear();
    ui->categories->addItems(builtins->getCategories());

    ui->categories->setCurrentRow(lastRowId);
}

void OBuiltInsEditor::setTable(int row_id)
{
    if (row_id < 0)
        return;

    lastRowId = row_id;

    blockEdits = true;

    const QLinkedList<QPair<unsigned, QString> > &values =
            builtins->getValues(ui->categories->item(row_id)->text());
    ui->values->clear();
    ids.clear();
    QLinkedList<QPair<unsigned, QString> >::const_iterator i;
    for (i = values.constBegin(); i != values.constEnd(); i++) {
        ui->values->addItem(i->second);
        /* Make item editable. */
        QListWidgetItem * item = ui->values->item(ui->values->count()-1);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ids[item] = i->first;
    }

    blockEdits = false;
}

void OBuiltInsEditor::addRow()
{
    builtins->addValue(ui->categories->item(lastRowId)->text());
}

void OBuiltInsEditor::removeRow()
{
    if (ui->categories->currentRow() < 0 || ui->values->currentRow() < 0)
        return;

    unsigned count = builtins->countSpecies(ui->categories->currentItem()->text(),
                                            ids[ui->values->currentItem()]);
    if (count)
        QMessageBox::warning(this, trUtf8("Remove value"),
                             trUtf8("Cannot remove value because it is used "
                                    "by %1 species.").arg(count));
    else
        builtins->removeValue(ui->categories->currentItem()->text(),
                              ids[ui->values->currentItem()]);
}

void OBuiltInsEditor::edit(QListWidgetItem *item)
{
    if (blockEdits)
        return;

    builtins->setValue(ui->categories->currentItem()->text(), ids[item], item->text());
}
