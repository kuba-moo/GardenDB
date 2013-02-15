#include "editor.h"
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

    builtinsEditor = 0;
    editor = 0;
    db = 0;

    connect(ui->actionNew_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionOpen_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionSpecies, SIGNAL(triggered()), SLOT(goToTable()));
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

    ui->actionSpecies->setEnabled(true);
    ui->actionEdit_built_ins->setEnabled(true);
    ui->actionClose_file->setEnabled(true);
    ui->actionSave->setEnabled(true);
    this->setWindowTitle(trUtf8("Garden") + " - " + QFileInfo(newFileName).baseName());

    goToTable();
}

void OMain::closeFile()
{
    goToTable();

    delete builtins;

    delete db;
    db = 0;

    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(false);
    ui->actionSpecies->setEnabled(false);
    ui->actionEdit_built_ins->setEnabled(false);
    ui->actionClose_file->setEnabled(false);
    ui->actionSave->setEnabled(false);
    this->setWindowTitle(trUtf8("Garden"));
}

void OMain::editBuiltIns()
{
    ui->actionEdit_built_ins->setChecked(true);
    ui->actionSpecies->setChecked(false);

    if (centralWidget() == builtinsEditor)
        return;

    builtinsEditor = new OBuiltInsEditor(builtins);
    connect(builtinsEditor, SIGNAL(finished()), SLOT(goToTable()));

    setCentralWidget(builtinsEditor);
}

void OMain::showDetails(QModelIndex index)
{
    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(false);

    if (centralWidget() == editor) {
        Log(Assert) << "OMain editor exists";
        return;
    }

    editor = new Editor(db, builtins, index, this);
    setCentralWidget(editor);
    connect(editor, SIGNAL(finished()), SLOT(goToTable()));
}

void OMain::goToTable()
{
    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(true);

    if (centralWidget() == mainTable)
        return;

    mainTable = new MainTable(db, this);
    connect(mainTable, SIGNAL(rowDetails(QModelIndex)),
            SLOT(showDetails(QModelIndex)));
    setCentralWidget(mainTable);
}
