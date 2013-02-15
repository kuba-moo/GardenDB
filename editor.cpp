#include "editor.h"
#include "builtins.h"
#include "database.h"
#include "image.h"
#include "imagecache.h"
#include "logger.h"
#include "specimen.h"
#include "specimenmodel.h"
#include "ui_editor.h"

#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

Editor::Editor(Database *db, BuiltIns *builtIns, const QModelIndex &index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Editor)
{
    ui->setupUi(this);

    ic = db->imageCache();
    connect(ic, SIGNAL(changed()), SLOT(reloadPhotos()));
    builtins = builtIns;
    specimen = (Specimen *)db->specimenModel()->data(index, Qt::DisplayRole).toULongLong();
    if (!specimen)
        Log(Assert) << "Editor wants to edit empty specimen";

    ui->title->setText(trUtf8("Edit ") + specimen->getName());

    ui->name->setText(specimen->getName());
    connect(ui->name, SIGNAL(textEdited(QString)), specimen, SLOT(setName(QString)));
    ui->grower->setText(specimen->getGrower());
    connect(ui->grower, SIGNAL(textEdited(QString)), specimen, SLOT(setGrower(QString)));
    ui->flowers->setText(specimen->getFlowers());
    connect(ui->flowers, SIGNAL(textEdited(QString)), specimen, SLOT(setFlowers(QString)));
    ui->bush->setText(specimen->getSize());
    connect(ui->bush, SIGNAL(textEdited(QString)), specimen, SLOT(setSize(QString)));
    ui->desc->setText(specimen->getDesc());
    connect(ui->desc, SIGNAL(textChanged()), SLOT(setDescription()));

    populateComboes();

    connect(builtins, SIGNAL(changed()), SLOT(populateComboes()));
    connect(ui->addPhoto, SIGNAL(clicked()), SLOT(addPhoto()));
    connect(ui->removePhoto, SIGNAL(clicked()), SLOT(removePhoto()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)),
            SLOT(setMainPhoto(int)));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(magnifyImage(QModelIndex)));
    connect(ui->backToTable, SIGNAL(clicked()), SIGNAL(finished()));

    reloadPhotos();
}

Editor::~Editor()
{
    delete ui;
}

void Editor::fillCombo(QComboBox *combo, const QString &category, const unsigned current)
{
    const QLinkedList<QPair<unsigned, QString> > &values = builtins->getValues(category);
    int currentIndex = -1;

    combo->clear();
    QLinkedList<QPair<unsigned, QString> >::const_iterator i;
    for (i = values.constBegin(); i != values.constEnd(); i++)
        if (i->first) {
            if (i->first == current)
                currentIndex = combo->count();

            combo->addItem(i->second, i->first);
        }
    combo->setCurrentIndex(currentIndex);

    connect(combo, SIGNAL(currentIndexChanged(int)), SLOT(handleCombo(int)));
}

void Editor::setDescription()
{
    specimen->setDesc(ui->desc->document()->toPlainText());
}

void Editor::populateComboes()
{
    fillCombo(ui->type, trUtf8("Types"), specimen->getTypeId());
    fillCombo(ui->flavour, trUtf8("Flavour"), specimen->getFlavourId());
    fillCombo(ui->flowering, trUtf8("Flowering time"), specimen->getFloweringId());
    fillCombo(ui->frost, trUtf8("Frost resistance"), specimen->getFrostId());
}

void Editor::handleCombo(int n)
{
    QComboBox *combo = dynamic_cast<QComboBox *>(QObject::sender());
    int id = combo->itemData(n).toInt();

    if (combo == ui->type)
        specimen->setTypeId(id);
    else if (combo == ui->flavour)
        specimen->setFlavourId(id);
    else if (combo == ui->flowering)
        specimen->setFloweringId(id);
    else if (combo == ui->frost)
        specimen->setFrostId(id);
    else
        Log(Assert) << "Editor handleCombo unknown combo";
}

void Editor::reloadPhotos()
{
    images = ic->getAllImages(specimen->getId());

    ui->listWidget->blockSignals(true);
    while (ui->listWidget->count())
        delete ui->listWidget->takeItem(0);
    ui->listWidget->blockSignals(false);

    int mainPhotoIndex = -1;
    QList<Image *>::const_iterator i;
    for (i = images.constBegin(); i < images.constEnd(); i++) {
        if ((*i)->id() == specimen->getMainPhotoId())
            mainPhotoIndex = ui->listWidget->count();
        QPixmap *pixmap = (*i)->getScaledGe(QSize(200, 200));
        ui->listWidget->addItem(new QListWidgetItem(QIcon(*pixmap), ""));
    }

    ui->listWidget->setCurrentRow(mainPhotoIndex);
}

void Editor::addPhoto()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, trUtf8("Select garden file"),
                                         QString(),
                                         trUtf8("Images (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;

    ic->addImage(specimen->getId(), fileName);
}

void Editor::removePhoto()
{
    int row = ui->listWidget->currentRow();
    if (row < 0)
        return;

    ic->removeImage(images[row]->id());
}

void Editor::setMainPhoto(int n)
{
    QLabel * const photo = ui->mainPhoto;

    if (n < 0) {
        photo->setPixmap(QIcon(":/icons/image").pixmap(photo->size()));
        specimen->setMainPhotoId(0);
    } else {
        photo->setPixmap(*images[n]->getScaled(photo->size()));
        specimen->setMainPhotoId(images[n]->id());
    }
}

void Editor::magnifyImage(QModelIndex index)
{
    QLabel *label = new QLabel("");
    label->setAttribute(Qt::WA_DeleteOnClose);

    QPixmap *pixmap = images[index.row()]->getScaledGe(QSize(800, 800));

    label->setPixmap(*pixmap);
    label->adjustSize();
    label->show();
}
