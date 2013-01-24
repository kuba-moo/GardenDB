#include "addnew.h"
#include "builtins.h"
#include "ui_addnew.h"
#include "oqueries.h"
#include "imagecache.h"

#include <QBuffer>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

AddNew::AddNew(ImageCache *imageCache, BuiltIns *builtIns, QWidget *parent,
               const QSqlRecord &record) :
    QWidget(parent),
    ui(new Ui::AddNew)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ic = imageCache;
    builtins = builtIns;

    resetData();
    if (!record.isEmpty())
        setData(record);
    populateComboes();

    connect(builtins, SIGNAL(changed()), SLOT(populateComboes()));
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

void AddNew::resetData()
{
    isNew = true;
    speciesId = typeId = flavourId = floweringId = frostId = 0;
}

void AddNew::setData(const QSqlRecord &record)
{
    int id = record.value("id").toInt();
    QSqlQuery bareRecord(QString("SELECT * FROM Species WHERE id = %1").arg(id));
    qDebug() << "Setting data for" << id;
    if (!bareRecord.next())
        return;

    isNew = false;
    speciesId = id;
    ui->title->setText(trUtf8("Edit ") + record.value("name").toString());
    ui->name->setText(record.value("name").toString());
    typeId = bareRecord.record().value("tp_id").toInt();
    ui->flowers->setText(record.value("flowers").toString());
    flavourId = bareRecord.record().value("fl_id").toInt();
    floweringId = bareRecord.record().value("fw_id").toInt();
    ui->bush->setText(record.value("size").toString());
    frostId = bareRecord.record().value("fr_id").toInt();
    ui->desc->setText(record.value("desc").toString());

    /* Pictures. */
    int mainPhotoIndex = -1;
    int mainPhotoId = bareRecord.record().value("main_photo").toInt();
    QSqlQuery pictures(QString("SELECT id FROM Images WHERE sp_id = %1")
                              .arg(speciesId));

    /* We could try to add some kind of prefetch here that would read all
     * specimen's pictures in one database access, but that would require
     * regerating images from bigger already in cache (we would prefetch
     * not-scaled) and somehow checking if images for specimen are not
     * already in cache (otherwise we would risk prefetch being a slowdown).
     *
     * It seem like a wiser idea to fetch images in query above and pass
     * QByteArrays into cache in case of a miss.
     *
     * Perhaps I should move this code over to ImgeCache and add getAllPictures
     * kind of function.
     */
    while (pictures.next())
    {
        const int picId = pictures.record().value("id").toInt();

        QPixmap *pixmap = ic->getPixmapGe(picId, QSize(200, 200));
        ui->listWidget->addItem(new QListWidgetItem(QIcon(*pixmap), ""));
        picIds.push_back(picId);
        if (mainPhotoId == picId)
            mainPhotoIndex = ui->listWidget->count()-1;
    }
    oldPhotoes = ui->listWidget->count();
    ui->listWidget->setCurrentRow(mainPhotoIndex);
    setMainPhoto(mainPhotoIndex);

    return;
}

void AddNew::populateComboes()
{
    fillCombo(ui->type, trUtf8("Types"), typeId);
    fillCombo(ui->flavour, trUtf8("Flavour"), flavourId);
    fillCombo(ui->flowering, trUtf8("Flowering time"), floweringId);
    fillCombo(ui->frost, trUtf8("Frost resistance"), frostId);
}

void AddNew::fillCombo(QComboBox *combo, const QString &category, const unsigned current)
{
    const QLinkedList<QPair<unsigned, QString> > &values = builtins->getValues(category);
    int currentIndex = 0;

    combo->clear();
    QLinkedList<QPair<unsigned, QString> >::const_iterator i;
    for (i = values.constBegin(); i != values.constEnd(); i++) {
        if (i->first == current)
            currentIndex = combo->count();

        combo->addItem(i->second, i->first);
    }
    combo->setCurrentIndex(currentIndex);
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

    if (! isNew)
    {
        QSqlQuery insertPicture;
        insertPicture.prepare("INSERT INTO Images VALUES(NULL, :image, :id)");

        QByteArray array;
        QBuffer buf(&array);
        buf.open(QIODevice::WriteOnly);
        pixmap.save(&buf, "PNG");

        insertPicture.bindValue(":id", speciesId);
        insertPicture.bindValue(":image", array);
        insertPicture.exec();

        picIds.push_back(insertPicture.lastInsertId().toInt());

        if (insertPicture.lastError().isValid())
            qDebug() << insertPicture.lastError();
    }

    QIcon icon(pixmap.scaled(QSize(800, 800), Qt::KeepAspectRatio,
                             Qt::SmoothTransformation));
    ui->listWidget->addItem(new QListWidgetItem(icon, ""));
    ui->listWidget->item(ui->listWidget->count()-1)->setToolTip(fileName);
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

    QPixmap *pixmap = ic->getPixmapGe(picIds[index.row()], QSize(800, 800));
    label->setPixmap(*pixmap);
    label->adjustSize();
    label->show();
}

void AddNew::acceptAdd()
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
        QPixmap pixmap;
        /* Check if image was read successfully. */
        if (! pixmap.load(item->toolTip())) {
            qDebug() << "Item is spooky!" << item;
            continue;
        }
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

void AddNew::acceptUpdate()
{
    int mainPhotoId = (ui->listWidget->currentRow() == -1) ?
                1 : picIds[ui->listWidget->currentRow()];

    QStringList ul;
    ul << "UPDATE Species SET";
    ul << " name='" << ui->name->text() << "'";
    ul << ", flowers='" << ui->flowers->text() << "'";
    ul << ", size='" << ui->bush->text() << "'";
    ul << ", main_photo='" << QString::number(mainPhotoId) << "'";
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
        qDebug() << updateQuery.executedQuery() << "\n" << updateQuery.lastError();
}

void AddNew::accept()
{
    if (isNew)
        acceptAdd();
    else
        acceptUpdate();

    close();
}
