#include "addnew.h"
#include "builtins.h"
#include "database.h"
#include "image.h"
#include "imagecache.h"
#include "ui_addnew.h"

#include <QBuffer>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

AddNew::AddNew(Database *db, BuiltIns *builtIns, QWidget *parent,
               const QSqlRecord &record) :
    QWidget(parent),
    ui(new Ui::AddNew)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ic = db->imageCache();
    connect(ic, SIGNAL(changed()), SLOT(reloadPhotos()));
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
    speciesId = -1;
    typeId = flavourId = floweringId = frostId = 0;
}

void AddNew::setData(const QSqlRecord &record)
{
    int id = record.value("id").toInt();
    QSqlQuery bareRecord(QString("SELECT * FROM Species WHERE id = %1").arg(id));
    qDebug() << "Setting data for" << id;
    if (!bareRecord.next())
        return;

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
    mainPhotoId = bareRecord.record().value("main_photo").toInt();
    reloadPhotos();
}

void AddNew::reloadPhotos()
{
    images = ic->getAllImages(speciesId);

    ui->listWidget->blockSignals(true);
    while (ui->listWidget->count())
        delete ui->listWidget->takeItem(0);
    ui->listWidget->blockSignals(false);

    int mainPhotoIndex = -1;
    QList<Image *>::const_iterator i;
    for (i = images.constBegin(); i < images.constEnd(); i++) {
        QPixmap *pixmap = (*i)->getScaledGe(QSize(200, 200));
        ui->listWidget->addItem(new QListWidgetItem(QIcon(*pixmap), ""));
        if ((*i)->id() == mainPhotoId)
            mainPhotoIndex = ui->listWidget->count()-1;
    }

    ui->listWidget->setCurrentRow(mainPhotoIndex);
    setMainPhoto(mainPhotoIndex);
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

void AddNew::addPhoto()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, trUtf8("Select garden file"),
                                         QString(),
                                         trUtf8("Images (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;

    ic->addImage(speciesId, fileName);
}

void AddNew::removePhoto()
{
    int row = ui->listWidget->currentRow();
    if (row < 0)
        return;

    ic->removeImage(images[row]->id());
}

void AddNew::setMainPhoto(int)
{
    /* TODO: implement AddNew::setMainPhoto */
#if 0
    QLabel * const photo = ui->mainPhoto;

    if (n < 0) {
        photo->setPixmap(QIcon(":/icons/image").pixmap(photo->size()));
        mainPhotoId = 1;
    } else {
        QListWidgetItem *selected = ui->listWidget->item(n);
        photo->setPixmap(selected->icon().pixmap(photo->size()));
        mainPhotoId = images[n]->id();
    }
#endif
}

void AddNew::magnifyImage(QModelIndex index)
{
    QLabel *label = new QLabel("");
    label->setAttribute(Qt::WA_DeleteOnClose);

    QPixmap *pixmap = images[index.row()]->getScaledGe(QSize(800, 800));

    label->setPixmap(*pixmap);
    label->adjustSize();
    label->show();
}

void AddNew::acceptAdd()
{
#if 0
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
#endif
}

void AddNew::acceptUpdate()
{
#if 0
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
#endif
}

void AddNew::accept()
{
    if (isNew())
        acceptAdd();
    else
        acceptUpdate();

    close();
}
