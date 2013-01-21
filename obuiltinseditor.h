#ifndef OBUILTINSEDITOR_H
#define OBUILTINSEDITOR_H

#include <QWidget>

namespace Ui {
    class OBuiltInsEditor;
}

class OBuiltInsEditor : public QWidget
{
    Q_OBJECT

public:
    explicit OBuiltInsEditor(QWidget *parent = 0);
    ~OBuiltInsEditor();

private slots:
    void changeTable(int);
    void addRow();
    void removeRow();

private:
    Ui::OBuiltInsEditor *ui;
};

#endif // OBUILTINSEDITOR_H
