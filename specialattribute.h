#ifndef SPECIALATTRIBUTE_H
#define SPECIALATTRIBUTE_H

#include <QWidget>

namespace Ui {
    class SpecialAttribute;
}

class SpecialAttribute : public QWidget
{
    Q_OBJECT

public:
    explicit SpecialAttribute(QString name,
                              QWidget *parent = 0);
    ~SpecialAttribute();

    void setId(int new_id) { _id = new_id; }
    int id() { return _id; }

    QString name();
    QString value();
    void setValue(QString newValue);

private:
    Ui::SpecialAttribute *ui;
    int _id;
};

#endif // SPECIALATTRIBUTE_H
