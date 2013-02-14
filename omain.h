#ifndef OMAIN_H
#define OMAIN_H

#include <QMainWindow>
#include <QModelIndex>

class Database;
class OBuiltInsEditor;
class MainTable;
class AddNew;
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

    /* Spawns row addition tab. Note: only one addition tab can be in use. */
    void addRow();
    /* Callback called on closing rowAdd. */
    void addRowClosed();

    /* Opens built-ins-editor or raise exisiting one. */
    void editBuiltIns();
    /* Callback registered on built-ins-editor destruction. */
    void builtInsClosed();

    /* Opens and sets focus to Garden (main tab). */
    void showGarden();

    /* Opens and sets focus to details page about selected row. */
    void showDetails(QModelIndex);

    /* Reloads data to table. */
    void refreshView();

private:
    /* Some file is currently opened. */
    bool isFileOpened() { return db; }

    Ui::OMain *ui;

    Database *db;
    OBuiltInsEditor* editor;

    /* Data objects */
    BuiltIns *builtins;

    /* Specific tabs */
    MainTable *mainTable;
    AddNew *addNew;
};

#endif // OMAIN_H
