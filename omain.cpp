#include "addnew.h"
#include "builtins.h"
#include "database.h"
#include "omain.h"
#include "ui_omain.h"
#include "obuiltinseditor.h"
#include "maintable.h"
#include "logger.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSqlRelationalTableModel>

OMain::OMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OMain)
{
    ui->setupUi(this);

    editor = 0;
    addNew = 0;
    db = 0;

    connect(ui->actionNew_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionOpen_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionEdit_built_ins,SIGNAL(triggered()),SLOT(editBuiltIns()));
    connect(ui->actionClose_file, SIGNAL(triggered()), SLOT(closeFile()));
    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

OMain::~OMain()
{
    if (isFileOpened())
        closeFile();

    delete ui;
}

void OMain::openFile()
{
    if (isFileOpened())
        closeFile();

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    QString newFileName =
            /* TODO: this should be getSaveFileName for windows. */
            fileDialog.getOpenFileName(this,
                                       trUtf8("Select garden file"),
                                       QString(),
                                       QString("Garden files (*.grd)"));
    if (newFileName.isEmpty())
        return;

    if (! newFileName.endsWith(".grd"))
        newFileName += ".grd";

    doOpen(newFileName);
}

void OMain::doOpen(QString newFileName)
{
    db = new Database(newFileName, this);
    if (!db->isOk()) {
        delete db;
        db = 0;
        return;
    }

    builtins = new BuiltIns(this);

    mainTable = new MainTable(db, builtins, this);
    connect(mainTable, SIGNAL(addRow()), SLOT(addRow()));
    connect(mainTable, SIGNAL(rowDetails(QModelIndex)),
            SLOT(showDetails(QModelIndex)));
    ui->tabWidget->addTab(mainTable, QIcon(), trUtf8("Garden"));

    ui->actionEdit_built_ins->setEnabled(true);
    this->setWindowTitle(trUtf8("Garden") + " - " + QFileInfo(newFileName).baseName());
}

void OMain::closeFile()
{
    while (ui->tabWidget->count())
    {
        QWidget *widget = ui->tabWidget->widget(ui->tabWidget->count()-1);
        ui->tabWidget->removeTab(ui->tabWidget->count()-1);
        widget->close();
        widget->deleteLater();
    }

    if (editor)
        editor->close();

    delete builtins;

    delete db;
    db = 0;

    ui->actionEdit_built_ins->setEnabled(false);
    this->setWindowTitle(trUtf8("Garden"));
}

void OMain::addRow()
{
    if (! addNew)
    {
        addNew = new AddNew(db, builtins, this);
        ui->tabWidget->addTab(addNew, QIcon(":/plus"), trUtf8("New specimen"));
    }

    ui->tabWidget->setCurrentWidget(addNew);
    connect(addNew, SIGNAL(destroyed()), SLOT(addRowClosed()));
}

void OMain::addRowClosed()
{
    addNew = 0;
    mainTable->loadData();
}

void OMain::editBuiltIns()
{
    if (! editor)
    {
        editor = new OBuiltInsEditor(builtins);

        connect(editor, SIGNAL(destroyed()), SLOT(builtInsClosed()));

        editor->show();
    }

    editor->raise();
}

void OMain::builtInsClosed()
{
    editor = 0;
}

void OMain::showGarden()
{
    if (mainTable)
        ui->tabWidget->setCurrentWidget(mainTable);
}

void OMain::showDetails(QModelIndex index)
{
    const QSqlRelationalTableModel *model =
            dynamic_cast<const QSqlRelationalTableModel *> (index.model());

    AddNew *tab = new AddNew(db, builtins, this, model->record(index.row()));
    ui->tabWidget->addTab(tab, QIcon(),
                          model->record(index.row()).value("name").toString());
    ui->tabWidget->setCurrentWidget(tab);
    connect(tab, SIGNAL(destroyed()), SLOT(refreshView()));
}

void OMain::refreshView()
{
    mainTable->loadData();
}
