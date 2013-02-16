#include "editor.h"
#include "database.h"
#include "gallery.h"
#include "omain.h"
#include "ui_omain.h"
#include "obuiltinseditor.h"
#include "maintable.h"
#include "logger.h"
#include "savingwidget.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlRelationalTableModel>

class Builtins;

OMain::OMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OMain)
{
    ui->setupUi(this);

    db = 0;

    Logger::instance()->setWindow(this);

    connect(ui->actionNew_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionOpen_file, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionClose_file, SIGNAL(triggered()), SLOT(closeFile()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(save()));
    connect(ui->actionSpecies, SIGNAL(triggered()), SLOT(goToTable()));
    connect(ui->actionEdit_built_ins,SIGNAL(triggered()),SLOT(editBuiltIns()));
    connect(ui->actionQuit, SIGNAL(triggered()), SLOT(quit()));

    if (QApplication::desktop()->width() < 1400)
        showMaximized();
}

OMain::~OMain()
{
    if (isFileOpened())
        doCloseFile();

    delete ui;
}

void OMain::doOpen(QString newFileName)
{
    QTime t; t.start();
    db = new Database(newFileName, this);
    if (!db->isOk()) {
        delete db;
        db = 0;
        return;
    }

    ui->actionSpecies->setEnabled(true);
    ui->actionEdit_built_ins->setEnabled(true);
    ui->actionClose_file->setEnabled(true);
    ui->actionSave->setEnabled(true);
    this->setWindowTitle(trUtf8("Garden") + " - " + QFileInfo(newFileName).baseName());

    goToTable();
    Log(Performance) << "Loading file" << newFileName << "took" << t.elapsed() << "ms";
}

void OMain::closeEvent(QCloseEvent *ev)
{
    if (centralWidget() == saving && saveReason != Done) {
        QMessageBox::information(this, trUtf8("Saving changes"),
                                 trUtf8("Saving changes in progress, please wait"
                                        " all changes are saved"));
        ev->ignore();
        return;
    }

    if (askSave(AppQuit))
        ev->accept();
    else
        ev->ignore();
}

bool OMain::askSave(SaveTrigger trigger)
{
    if (!db || !db->isModified())
        return true;

    int ret = QMessageBox::question(this, trUtf8("Save file"),
                                    trUtf8("Do you want to save changes?"),
                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                    QMessageBox::Save);
    if (ret == QMessageBox::Cancel)
        return false;
    if (ret == QMessageBox::Save) {
        save(trigger);
        return false;
    }
    return true;
}

void OMain::quit()
{
    if (askSave(AppQuit))
        qApp->quit();
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

void OMain::closeFile()
{
    if (askSave(FileClose))
        doCloseFile();
}

void OMain::doCloseFile()
{
    goToTable();

    setCentralWidget(new QWidget(this));
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

void OMain::save(SaveTrigger reason)
{
    saveReason = reason;

    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(false);
    ui->actionSave->setChecked(true);

    ui->actionNew_file->setEnabled(false);
    ui->actionOpen_file->setEnabled(false);
    ui->actionClose_file->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionSpecies->setEnabled(false);
    ui->actionEdit_built_ins->setEnabled(false);
    ui->actionQuit->setEnabled(false);

    saving = new SavingWidget(db, this);
    connect(saving, SIGNAL(done()), SLOT(savingDone()));
    connect(saving, SIGNAL(finished()), SLOT(goToTable()));
    setCentralWidget(saving);
}

void OMain::savingDone()
{
    switch (saveReason) {
    case Done: Log(Assert) << "Saving double done-done."; break;
    case Action: saveReason = Done; break;
    case FileClose: doCloseFile(); break;
    case AppQuit: qApp->quit(); break;
    }

    ui->actionNew_file->setEnabled(true);
    ui->actionOpen_file->setEnabled(true);
    ui->actionClose_file->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSpecies->setEnabled(true);
    ui->actionEdit_built_ins->setEnabled(true);
    ui->actionQuit->setEnabled(true);
}

void OMain::editBuiltIns()
{
    ui->actionEdit_built_ins->setChecked(true);
    ui->actionSpecies->setChecked(false);
    ui->actionSave->setChecked(false);

    if (centralWidget() == builtinsEditor)
        return;

    builtinsEditor = new OBuiltInsEditor(db->builtins());
    connect(builtinsEditor, SIGNAL(finished()), SLOT(goToTable()));
    setCentralWidget(builtinsEditor);
}

void OMain::showGallery(const QModelIndex &index)
{
    showGallery((Specimen *)index.data().toULongLong());
}

void OMain::showGallery(Specimen *s)
{
    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(false);

    Gallery *gallery = new Gallery(db, s, this);
    connect(gallery, SIGNAL(finished()), SLOT(goToTable()));
    setCentralWidget(gallery);
}

void OMain::showDetails(const QModelIndex &index)
{
    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(false);

    editor = new Editor(db, index, this);
    connect(editor, SIGNAL(requestGallery(Specimen*)),
            SLOT(showGallery(Specimen*)));
    connect(editor, SIGNAL(finished()), SLOT(goToTable()));
    setCentralWidget(editor);
}

void OMain::goToTable()
{
    ui->actionEdit_built_ins->setChecked(false);
    ui->actionSpecies->setChecked(true);
    ui->actionSave->setChecked(false);

    if (centralWidget() == mainTable)
        return;

    mainTable = new MainTable(db, this);
    connect(mainTable, SIGNAL(requestEditor(QModelIndex)),
            SLOT(showDetails(QModelIndex)));
    connect(mainTable, SIGNAL(requestGallery(QModelIndex)),
            SLOT(showGallery(QModelIndex)));
    setCentralWidget(mainTable);
}
