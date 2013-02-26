#ifndef OMAIN_H
#define OMAIN_H

#include <QMainWindow>
#include <QModelIndex>

class Database;
class OBuiltInsEditor;
class MainTable;
class Editor;
class SavingWidget;
class Specimen;

namespace Ui {
    class OMain;
}

class OMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit OMain(QWidget *parent = 0);
    ~OMain();

    /* Open file from given path. */
    void doOpen(QString);

protected:
    void closeEvent(QCloseEvent *);

    /* All actions that may cause closing current file need to be split into
     * at least three separate stages:
     * action trigger handler -> ask user about saving [ -> perform save ] -> exec action
     *                                               ||  -> cancel == do nothing
     *
     * Actual control flow is like this:
     * startHandle -> userSave [cancel] -> do nothing
     *                         [discard] -> dispatchAction
     *                         [not modified] -> dispatchAction
     *                         [save] -> save ... savingDone -> dispatchAction
     */
    enum SaveTrigger {
        Done,
        ActionSave,
        FileNew,
        FileOpen,
        FileClose,
        AppQuit
    };

private slots:
    /* First-stage handler for all action that may cause closing of current file. */
    void handleActionMaySave();

    /* User-triggered saving. */
    void triggerSave();
    /* Saving process finished, clean up. */
    void savingDone();
    /* Opens built-ins-editor. */
    void editBuiltIns();
    /* Opens and sets focus to gallery page of selected row. */
    void showGallery(Specimen *s);
    void showGallery(const QModelIndex &index);
    /* Opens and sets focus to details page about selected row. */
    void showDetails(Specimen *s);
    void showDetails(const QModelIndex &index);
    /* Handle destruction of central widget. */
    void goToTable();
    /* Attempt to quit application. */
    void quit();
    /* Ask user if she wants to save changes and continue with the process
     * according to trigger. */
    void userSave(SaveTrigger trigger);

private:
    /* Opens saving widget and start saving process. */
    void save();
    /* Shows file selection dialog and load selected file. If another file
     * is currently open it will call closeFile() to close it. May be used
     * to both open existing files or create new ones. After file is loaded
     * main tab should be opened and presented to user. */
    void openFile();
    /* Attempt to close currently opened file. */
    void doCloseFile();
    /* Final stage of action involvng saving file - do actual thing requested. */
    void dispatchAction();
    /* Some file is currently opened. */
    bool isFileOpened() { return db; }

    Database *db;

    /* Specific tabs - removed when setCentralWidget changes widgets. */
    MainTable *mainTable;
    Editor *editor;
    OBuiltInsEditor *builtinsEditor;
    SavingWidget *saving;

    SaveTrigger saveReason;
    void saveTablePos();
    int prevTableScroll, prevTableSel;

    Ui::OMain *ui;
};

#endif // OMAIN_H
