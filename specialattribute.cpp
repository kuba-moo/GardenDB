#include "specialattribute.h"
#include "ui_specialattribute.h"

SpecialAttribute::SpecialAttribute(QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpecialAttribute)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    _id = 0;
    ui->name->setText(name + ":\t");

    connect(ui->remove, SIGNAL(clicked()), SLOT(close()));
}

SpecialAttribute::~SpecialAttribute()
{
    delete ui;
}

QString SpecialAttribute::name()
{
    return ui->name->text().remove(":\t");
}

QString SpecialAttribute::value()
{
    return ui->value->text();
}

void SpecialAttribute::setValue(QString newValue)
{
    ui->value->setText(newValue);
}
