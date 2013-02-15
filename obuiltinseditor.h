#ifndef OBUILTINSEDITOR_H
#define OBUILTINSEDITOR_H

#include <QMap>
#include <QWidget>

namespace Ui {
    class OBuiltInsEditor;
}

class BuiltIns;
class QListWidgetItem;

class OBuiltInsEditor : public QWidget
{
    Q_OBJECT

public:
    explicit OBuiltInsEditor(BuiltIns *builtIns, QWidget *parent = 0);
    ~OBuiltInsEditor();

signals:
    void finished();

private slots:
    /* Reload tables and values from BuiltIns. */
    void reload();
    /* Set table id and display it's values. */
    void setTable(int row_id = 0);
    void addRow();
    void removeRow();
    /* Send information about edit to BuiltIns to be saved in database. */
    void edit(QListWidgetItem*);

private:
    Ui::OBuiltInsEditor *ui;
    int lastRowId;
    BuiltIns *builtins;
    QMap<QListWidgetItem *, unsigned> ids;
    /* Indicate that ui->values are modified programaticaly. */
    bool blockEdits;
};

#endif // OBUILTINSEDITOR_H
