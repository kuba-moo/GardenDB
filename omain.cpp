#include "addnew.h"
#include "builtins.h"
#include "omain.h"
#include "ui_omain.h"
#include "oqueries.h"
#include "obuiltinseditor.h"
#include "maintable.h"
#include "imagecache.h"

#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QIcon>

OMain::OMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OMain)
{
    ui->setupUi(this);

    editor = 0;
    addNew = 0;
    database = QSqlDatabase::addDatabase("QSQLITE");

    connect(ui->actionNew_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionOpen_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionEdit_built_ins,SIGNAL(triggered()),SLOT(editBuiltIns()));
    connect(ui->actionClose_file, SIGNAL(triggered()), SLOT(closeFile()));
    connect(ui->actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

OMain::~OMain()
{
    if (! fileName.isEmpty())
        closeFile();

    delete ui;
}

void OMain::openFile()
{
    if (! fileName.isEmpty())
        closeFile();

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    QString newFileName =
            fileDialog.getOpenFileName(this,
                                       trUtf8("Select garden file"),
                                       QString(),
                                       QString("Garden files (*.grd)"));
    if (newFileName.isEmpty())
        return;

    doOpen(newFileName);
}

void OMain::doOpen(QString newFileName)
{
    fileName = newFileName;
    if (! fileName.endsWith(".grd"))
        fileName += ".grd";

    database.setDatabaseName(fileName);
    database.open();

    QSqlQuery result;
    for (unsigned i=0; i < numCreates && !result.lastError().isValid(); i++)
        result = database.exec(creates[i]);

    if (result.lastError().isValid())
    {
        QMessageBox::critical(this, trUtf8("Unable to open file"),
                              trUtf8("Selected file cannot be opened"));
        qDebug() << result.lastQuery();
        qDebug() << result.lastError().text();
    }

    for (unsigned i=0; i < numInserts && !result.lastError().isValid(); i++)
        result = database.exec(inserts[i]);

    builtins = new BuiltIns;
    ic = new ImageCache;

    mainTable = new MainTable(ic, this);
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
    delete ic;

    database.close();

    ui->actionEdit_built_ins->setEnabled(false);

    this->setWindowTitle(trUtf8("Garden"));
}

void OMain::addRow()
{
    if (! addNew)
    {
        addNew = new AddNew(ic, this);
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
        editor = new OBuiltInsEditor();

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

    AddNew *tab = new AddNew(ic, this);
    ui->tabWidget->addTab(tab, QIcon(),
                          model->record(index.row()).value("name").toString());
    tab->setData(model->record(index.row()));
    ui->tabWidget->setCurrentWidget(tab);
    connect(tab, SIGNAL(destroyed()), SLOT(refreshView()));
}

void OMain::refreshView()
{
    mainTable->loadData();
}
