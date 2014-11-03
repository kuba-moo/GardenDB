/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "builtins.h"
#include "obuiltinseditor.h"
#include "ui_obuiltinseditor.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlTableModel>

OBuiltInsEditor::OBuiltInsEditor(Builtins *builtIns, QWidget *parent) :
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
    const QLinkedList<BuiltinValue *> &values =
            builtins->getValuesTr(ui->categories->item(row_id)->text());
    ui->values->clear();
    ids.clear();
    QLinkedList<BuiltinValue *>::const_iterator i;
    for (i = values.constBegin(); i != values.constEnd(); i++) {
        ui->values->addItem((*i)->value());
        /* Make item editable. */
        QListWidgetItem * item = ui->values->item(ui->values->count()-1);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ids[item] = (*i)->id();
    }
    blockEdits = false;
}

void OBuiltInsEditor::addRow()
{
    builtins->addValueTr(ui->categories->item(lastRowId)->text());
    int rows = ui->values->model()->rowCount();
    ui->values->edit(ui->values->model()->index(rows-1, 0));
}

void OBuiltInsEditor::removeRow()
{
    if (ui->categories->currentRow() < 0 || ui->values->currentRow() < 0)
        return;

    int ret = QMessageBox::question(this, trUtf8("Remove row"),
                                    trUtf8("Sure you want to remove this row?"),
                                    QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::Yes)
        builtins->removeValueTr(ui->categories->currentItem()->text(),
                                ids[ui->values->currentItem()]);
}

void OBuiltInsEditor::edit(QListWidgetItem *item)
{
    if (blockEdits)
        return;

    builtins->setValueTr(ui->categories->currentItem()->text(), ids[item], item->text());
}
