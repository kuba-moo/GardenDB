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

    enum SaveTrigger {
        Done,
        Action,
        FileClose,
        AppQuit
    };

private slots:
    /* Shows file selection dialog and load selected file. If another file
     * is currently open it will call closeFile() to close it. May be used
     * to both open existing files or create new ones. After file is loaded
     * main tab should be opened and presented to user. */
    void openFile();
    /* Close currently opened files, all the tabs and built-ins editors
     * associated with them. */
    void closeFile();
    void doCloseFile();

    /* Opens saving widget. */
    void save(SaveTrigger trigger = Action);
    /* Saving process finished, clean up. */
    void savingDone();
    /* Opens built-ins-editor. */
    void editBuiltIns();
    /* Opens and sets focus to gallery page of selected row. */
    void showGallery(Specimen *s);
    void showGallery(const QModelIndex &index);
    /* Opens and sets focus to details page about selected row. */
    void showDetails(const QModelIndex &index);
    /* Handle destruction of central widget. */
    void goToTable();
    /* Attempt to quit application. */
    void quit();
    /* Ask user if she wants to save changes, returns true if caller can
     * proceed i.e. user don't want to save changes. */
    bool askSave(SaveTrigger trigger);

private:
    /* Some file is currently opened. */
    bool isFileOpened() { return db; }

    Database *db;

    /* Specific tabs - removed when setCentralWidget changes widgets. */
    MainTable *mainTable;
    Editor *editor;
    OBuiltInsEditor *builtinsEditor;
    SavingWidget *saving;

    SaveTrigger saveReason;

    Ui::OMain *ui;
};

#endif // OMAIN_H
