#ifndef OBUILTINSEDITOR_H
#define OBUILTINSEDITOR_H

#include <QWidget>
#include <QSqlDatabase>

namespace Ui {
    class OBuiltInsEditor;
}

class OBuiltInsEditor : public QWidget
{
    Q_OBJECT

public:
    explicit OBuiltInsEditor(QSqlDatabase& db, QWidget *parent = 0);
    ~OBuiltInsEditor();

private slots:
    void changeTable(int);
    void addRow();
    void removeRow();

private:
    QSqlDatabase& database;
    Ui::OBuiltInsEditor *ui;
};

#endif // OBUILTINSEDITOR_H
