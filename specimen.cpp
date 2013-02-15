#include "specimen.h"

#include <QDebug>
#include <QVariant>

Specimen::Specimen(const int _id, const int _no, QObject *parent) :
    QObject(parent)
{
    state = New;

    id = _id;
    no = _no;
    name = grower = flowers = size = desc = QString();
    main_photo = fl_id = fw_id = fr_id = tp_id = 0;
}

Specimen::Specimen(QSqlRecord rec, QObject *parent) :
    QObject(parent)
{
    state = Unmodified;

    id = rec.value("id").toInt();
    no = rec.value("no").toInt();
    name = rec.value("name").toString();
    grower = rec.value("grower").toString();
    flowers = rec.value("flowers").toString();
    size = rec.value("size").toString();
    desc = rec.value("desc").toString();
    main_photo = rec.value("main_photo").toInt();
    fl_id = rec.value("fl_id").toInt();
    fw_id = rec.value("fw_id").toInt();
    fr_id = rec.value("fr_id").toInt();
    tp_id = rec.value("tp_id").toInt();
}

void Specimen::markModified()
{
    if (state == Unmodified) {
        state = Modified;
        qDebug() << "Specimen" << id << "changed";
    }
}


#if 0
void Editor::acceptAdd()
{
    QString insert = "INSERT INTO Species VALUES("
            "NULL, '" + ui->name->text() + "', '" + ui->flowers->text() + "', "
            "'" + ui->bush->text() + "', '"
            + ui->desc->document()->toPlainText()
            +"',0 , "
            + ui->flavour->itemData(ui->flavour->currentIndex()).toString()
            + ", "
            + ui->flowering->itemData(ui->flowering->currentIndex()).toString()
            + ", "
            + ui->frost->itemData(ui->frost->currentIndex()).toString() + ", "
            + ui->type->itemData(ui->type->currentIndex()).toString() + ")";
    QSqlQuery add(insert);
    if (add.lastError().isValid())
        qDebug() << __FILE__ << add.lastError();

    speciesId = add.lastInsertId().toInt();

    while (invalid.count())
        delete invalid.takeLast();
    for (int i = 0; i < images.count(); i++)
        images[i]->setOwner(speciesId);

    if (ui->listWidget->currentRow() >= 0)
        images[ui->listWidget->currentRow()]->mainPhoto();
    ic->setImages(speciesId, images, invalid);
}

void Editor::acceptUpdate()
{
    QStringList ul;
    ul << "UPDATE Species SET";
    ul << " name='" << ui->name->text() << "'";
    ul << ", flowers='" << ui->flowers->text() << "'";
    ul << ", size='" << ui->bush->text() << "'";
    ul << ", main_photo='" << QString::number(abs(mainPhotoId)) << "'";
    ul << ", desc='" << ui->desc->document()->toPlainText() << "'";
    ul << ", fl_id="
       << QString::number(ui->flavour->itemData(ui->flavour->currentIndex()).toInt());
    ul << ", fw_id="
       << QString::number(ui->flowering->itemData(ui->flowering->currentIndex()).toInt());
    ul << ", fr_id="
       << QString::number(ui->frost->itemData(ui->frost->currentIndex()).toInt());
    ul << ", tp_id="
       << QString::number(ui->type->itemData(ui->type->currentIndex()).toInt());
    ul << " WHERE id=" << QString::number(speciesId);

    QSqlQuery updateQuery(ul.join(""));
    if (updateQuery.lastError().isValid())
        qDebug() << updateQuery.lastError().text();

    if (ui->listWidget->currentRow() >= 0)
        images[ui->listWidget->currentRow()]->mainPhoto();
    ic->setImages(speciesId, images, invalid);
}
#endif
