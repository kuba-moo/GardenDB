#ifndef OMAIN_H
#define OMAIN_H

#include <QMainWindow>
#include <QModelIndex>

class Database;
class OBuiltInsEditor;
class MainTable;
class Editor;
class BuiltIns;

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

private slots:
    /* Shows file selection dialog and load selected file. If another file
     * is currently open it will call closeFile() to close it. May be used
     * to both open existing files or create new ones. After file is loaded
     * main tab should be opened and presented to user. */
    void openFile();
    /* Close currently opened files, all the tabs and built-ins editors
     * associated with them. */
    void closeFile();

    /* Opens built-ins-editor or raise exisiting one. */
    void editBuiltIns();
    /* Opens and sets focus to details page about selected row. */
    void showDetails(QModelIndex);
    /* Handle destruction of central widget. */
    void goToTable();

private:
    /* Some file is currently opened. */
    bool isFileOpened() { return db; }

    Ui::OMain *ui;

    /* Data objects */
    BuiltIns *builtins;
    Database *db;

    /* Specific tabs - removed when setCentralWidget changes widgets. */
    MainTable *mainTable;
    Editor *editor;
    OBuiltInsEditor* builtinsEditor;
};

#endif // OMAIN_H
