#include "addnew.h"
#include "ui_addnew.h"
#include "oqueries.h"
#include "specialattribute.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QBuffer>

AddNew::AddNew(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddNew)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    isNew = true;
    populateComboes();

    connect(ui->addPhoto, SIGNAL(clicked()), SLOT(addPhoto()));
    connect(ui->removePhoto, SIGNAL(clicked()), SLOT(removePhoto()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)),
            SLOT(setMainPhoto(int)));
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), SLOT(close()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(magnifyImage(QModelIndex)));
}

AddNew::~AddNew()
{
    delete ui;
}

/* TODO: this is slow, we can't just query it every time. */
void AddNew::populateComboes()
{
    QSqlQuery query(getTypes);
    ui->type->clear();
    while (query.next())
        ui->type->addItem(query.value(1).toString(), query.value(0));

    query.exec(getFlavours);
    ui->flavour->clear();
    while (query.next())
        ui->flavour->addItem(query.value(1).toString(), query.value(0));

    query.exec(getFlowering);
    ui->flowering->clear();
    while (query.next())
        ui->flowering->addItem(query.value(1).toString(), query.value(0));

    query.exec(getFrost);
    ui->frost->clear();
    while (query.next())
        ui->frost->addItem(query.value(1).toString(), query.value(0));
}

void AddNew::setData(const QSqlRecord &record)
{
    isNew = false;

    speciesId = record.value("id").toInt();
    QSqlQuery bareRecord(QString("SELECT * FROM Species WHERE id = %1")
                         .arg(speciesId));
    if (! bareRecord.next())
        return;

    ui->title->setText(trUtf8("Edit ") + record.value("name").toString());
    ui->name->setText(record.value("name").toString());
    ui->type->setCurrentIndex(bareRecord.record().value("tp_id").toInt()-1);
    ui->flowers->setText(record.value("flowers").toString());
    ui->flavour->setCurrentIndex(bareRecord.record().value("fl_id").toInt()-1);
    ui->flowering->setCurrentIndex(
                bareRecord.record().value("fw_id").toInt()-1);
    ui->bush->setText(record.value("size").toString());
    ui->frost->setCurrentIndex(bareRecord.record().value("fr_id").toInt()-1);
    ui->desc->setText(record.value("desc").toString());

    QSqlQuery pictures(QString("SELECT * FROM Images WHERE sp_id = %1")
                       .arg(speciesId));
    int mainPhotoId = bareRecord.record().value("main_photo").toInt();
    int mainPhotoIndex = -1;
    while (pictures.next())
    {
        QByteArray data(pictures.record().value("data").toByteArray());
        QPixmap pixmap;
        pixmap.loadFromData(data, "PNG");
        ui->listWidget->addItem(new QListWidgetItem(QIcon(pixmap), ""));
        picIds.push_back(pictures.record().value("id").toInt());
        if (mainPhotoId == pictures.record().value("id").toInt())
            mainPhotoIndex = ui->listWidget->count()-1;
    }
    oldPhotoes = ui->listWidget->count();
    ui->listWidget->setCurrentRow(mainPhotoIndex);

    /* TODO: really remove special attributes from project. */
    return;
/*
    QSqlQuery attributes(QString("SELECT * FROM Attributes WHERE sp_id = %1")
                         .arg(speciesId));
    while (attributes.next())
    {
        SpecialAttribute *attr = new SpecialAttribute(
                    attributes.record().value("name").toString(), this);
        attr->setValue(attributes.record().value("value").toString());
        attr->setId(attributes.record().value("id").toInt());

        specialAttributes.append(attr);
        connect(attr, SIGNAL(destroyed()), SLOT(removeAttribute()));

        ui->formLayout->addRow(attr);
    }*/
}

void AddNew::removePhoto()
{
    int row = ui->listWidget->currentRow();
    if (row < 0)
        return;

    if (! isNew)
    {
        QSqlQuery remove(QString("DELETE FROM Images WHERE id = %1")
                         .arg(picIds[row]));
        if (remove.numRowsAffected() != 1)
            QMessageBox::critical(this, trUtf8("Removing failed"),
                                  trUtf8("Unspecified error occured during "
                                         "attempt to remove picture.\n"
                                         "Please cancel edtition, reopen "
                                         "tab and try again."));
        picIds.remove(row);
    }

    delete ui->listWidget->takeItem(row);
}

void AddNew::addPhoto()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, trUtf8("Select garden file"),
                                         QString(),
                                         trUtf8("Images (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;

    QPixmap pixmap;
    /* Check if image was read successfully. */
    if (! pixmap.load(fileName))
        return;

    QIcon image(pixmap);

    if (! isNew)
    {
        QSqlQuery insertPicture;
        insertPicture.prepare("INSERT INTO Images VALUES(NULL, :image, :id)");

        QByteArray array;
        QBuffer buf(&array);
        QPixmap pixmap = image.pixmap(ImageSize.width(), ImageSize.height());
        buf.open(QIODevice::WriteOnly);
        pixmap.save(&buf, "PNG");

        insertPicture.bindValue(":id", speciesId);
        insertPicture.bindValue(":image", array);
        insertPicture.exec();

        picIds.push_back(insertPicture.lastInsertId().toInt());

        if (insertPicture.lastError().isValid())
            qDebug() << insertPicture.lastError();
    }

    ui->listWidget->addItem(new QListWidgetItem(image, QString()));
}

void AddNew::setMainPhoto(int n)
{
    QLabel * const photo = ui->mainPhoto;

    if (n < 0)
        photo->setPixmap(QIcon(":/icons/image").pixmap(photo->size()));
    else {
        QListWidgetItem *selected = ui->listWidget->item(n);
        photo->setPixmap(selected->icon().pixmap(photo->size()));
    }
}

void AddNew::magnifyImage(QModelIndex index)
{
    QLabel *label = new QLabel("");
    label->setAttribute(Qt::WA_DeleteOnClose);
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    label->setPixmap(icon.pixmap(800));
    label->adjustSize();
    label->show();
}

void AddNew::accept()
{
    /* TODO: marge some things maybe? */
    if (isNew)
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

    QListWidgetItem *item;
    QSqlQuery insertPicture;
    int mainPhotoId = 1;
    insertPicture.prepare("INSERT INTO Images VALUES(NULL, :image, :id)");
    for (int i=0; i < ui->listWidget->count(); i++)
    {
        item = ui->listWidget->item(i);

        QByteArray array;
        QBuffer buf(&array);
        QPixmap pixmap = item->icon().pixmap(ImageSize.width(),
                                             ImageSize.height());
        buf.open(QIODevice::WriteOnly);
        pixmap.save(&buf, "PNG");

        insertPicture.bindValue(":id", speciesId);
        insertPicture.bindValue(":image", array);
        insertPicture.exec();

        if (insertPicture.lastError().isValid())
            qDebug() << insertPicture.lastError();

        if (i == ui->listWidget->currentRow())
            mainPhotoId = insertPicture.lastInsertId().toInt();
    }

    QSqlQuery setMain(QString("UPDATE Species SET main_photo=%1 WHERE id=%2")
                      .arg(mainPhotoId).arg(speciesId));

    if (setMain.lastError().isValid())
        qDebug() << setMain.lastError();
    }
    else
    {
        QSqlQuery update(QString("UPDATE Species SET %1 = '%2' WHERE id=%3")
                         .arg("name").arg(ui->name->text()).arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = '%2' WHERE id=%3")
                    .arg("flowers").arg(ui->flowers->text()).arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = '%2' WHERE id=%3")
                    .arg("size").arg(ui->bush->text()).arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        int mainPhotoId = (ui->listWidget->currentRow() == -1) ?
                    1 : picIds[ui->listWidget->currentRow()];
        update.exec(QString("UPDATE Species SET %1 = '%2' WHERE id=%3")
                    .arg("main_photo").arg(mainPhotoId).arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = '%2' WHERE id=%3")
                    .arg("desc").arg(ui->desc->document()->toPlainText())
                    .arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = %2 WHERE id=%3")
                    .arg("fl_id")
                    .arg(ui->flavour->itemData(
                             ui->flavour->currentIndex()).toInt())
                    .arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = %2 WHERE id=%3")
                    .arg("fw_id")
                    .arg(ui->flowering->itemData(
                             ui->flowering->currentIndex()).toInt())
                    .arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = %2 WHERE id=%3")
                    .arg("fr_id")
                    .arg(ui->frost->itemData(
                             ui->frost->currentIndex()).toInt())
                    .arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();

        update.exec(QString("UPDATE Species SET %1 = %2 WHERE id=%3")
                    .arg("tp_id")
                    .arg(ui->type->itemData(
                             ui->type->currentIndex()).toInt())
                    .arg(speciesId));
        if (update.lastError().isValid())
            qDebug() << update.executedQuery() << "\n" << update.lastError();
    }

    QSqlQuery insertAttribute;
    insertAttribute.prepare(QString("INSERT INTO Attributes VALUES("
                                      "NULL, %1, :value, :name)")
                              .arg(speciesId));
    foreach (SpecialAttribute *sa, specialAttributes) {
        if (sa->id())
            continue;

        insertAttribute.bindValue(":value", sa->value());
        insertAttribute.bindValue(":name", sa->name());
        insertAttribute.exec();

        if (insertAttribute.lastError().isValid())
            qDebug() << __FILE__ << insertAttribute.executedQuery() << '\n'
                        << insertAttribute.lastError();
    }

    close();
}
