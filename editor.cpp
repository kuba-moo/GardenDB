#include "editor.h"
#include "builtins.h"
#include "database.h"
#include "image.h"
#include "imagecache.h"
#include "imagelistmodel.h"
#include "imagelistrenderer.h"
#include "logger.h"
#include "specimen.h"
#include "specimenmodel.h"
#include "ui_editor.h"

#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>

Editor::Editor(Database *db, Specimen *s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Editor)
{
    ui->setupUi(this);

    //connect(ic, SIGNAL(changed()), SLOT(reloadPhotos()));
    builtins = db->builtins();
    specimen = s;
    if (!specimen)
        Log(Assert) << "Editor wants to edit empty specimen";

    ui->title->setText(specimen->getName());
    ImageListModel *iml = new ImageListModel(db->imageCache(),
                                             specimen->getId(), this);
    ui->listView->setModel(iml);
    connect(iml, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(imageChanged(QModelIndex,QModelIndex)));
    ui->listView->setItemDelegate(new ImageListRenderer(100, this));

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
    connect(ui->name, SIGNAL(textEdited(QString)), ui->title, SLOT(setText(QString)));

    populateComboes();

    connect(builtins, SIGNAL(changed()), SLOT(populateComboes()));
    connect(ui->addPhoto, SIGNAL(clicked()), SLOT(addPhoto()));
    connect(ui->removePhoto, SIGNAL(clicked()), SLOT(removePhoto()));
    connect(ui->listView, SIGNAL(clicked(QModelIndex)),
            SLOT(setMainPhoto(QModelIndex)));
    connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(emitRequestGallery()));
    connect(ui->gallery, SIGNAL(clicked()), SLOT(emitRequestGallery()));
    connect(ui->mainPhoto, SIGNAL(clicked()),
            SLOT(mainPhotoClicked()));
    connect(ui->backToTable, SIGNAL(clicked()), SIGNAL(finished()));

    reloadPhotos();

    ui->listView->setAcceptDrops(true);
}

Editor::~Editor()
{
    delete ui;
}

void Editor::fillCombo(QComboBox *combo, const QString &category, const int current)
{
    const QLinkedList<BuiltinValue *> &values = builtins->getValues(category);
    int currentIndex = -1;

    combo->clear();
    QLinkedList<BuiltinValue *>::const_iterator i;
    for (i = values.constBegin(); i != values.constEnd(); i++) {
        if ((*i)->id() == current)
            currentIndex = combo->count();

        combo->addItem((*i)->value(), (*i)->id());
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
    fillCombo(ui->type, "Types", specimen->getTypeId());
    fillCombo(ui->flavour, "Flavour", specimen->getFlavourId());
    fillCombo(ui->flowering, "Flowering", specimen->getFloweringId());
    fillCombo(ui->frost, "Frost", specimen->getFrostId());
}

void Editor::handleCombo(int n)
{
    QComboBox *combo = qobject_cast<QComboBox *>(QObject::sender());
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
    QAbstractItemModel *model = ui->listView->model();

    for (int i = 0; i < model->rowCount(); i++) {
        Image *img = (Image *)model->data(model->index(i, 0)).toULongLong();
        if (img->id() == specimen->getMainPhotoId()) {
            setMainPhoto(model->index(i, 0));
            break;
        }
    }

    if (!ui->listView->model()->rowCount())
        ui->gallery->setEnabled(false);
}

void Editor::imageChanged(const QModelIndex &index,const QModelIndex &)
{
    QModelIndex current = ui->listView->selectionModel()->currentIndex();

    if (!current.isValid())
        setMainPhoto(ui->listView->model()->index(0, 0));
    else if (current.row() == index.row())
        setMainPhoto(current);

    if (!ui->listView->model()->rowCount())
        ui->gallery->setEnabled(false);
    else
        ui->gallery->setEnabled(true);
}

void Editor::addPhoto()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, trUtf8("Select image to add"),
                                         QString(),
                                         trUtf8("Images (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;

    ui->listView->model()->setData(QModelIndex(), fileName);

    QModelIndex current = ui->listView->selectionModel()->currentIndex();
    if (!current.isValid())
        setMainPhoto(ui->listView->model()->index(0, 0));

    ui->gallery->setEnabled(true);
}

void Editor::removePhoto()
{    
    QModelIndex current = ui->listView->selectionModel()->currentIndex();
    const int row = current.row();
    Image *img = (Image *)current.data().toULongLong();
    if (!img)
        return;

    ui->listView->model()->removeRows(current.row(), 1, current.parent());

    if (row > 0)
        setMainPhoto(ui->listView->model()->index(row-1, 0));
    else if (row < ui->listView->model()->rowCount())
        setMainPhoto(ui->listView->model()->index(row, 0));
    else {
        setMainPhoto(QModelIndex());
        ui->gallery->setEnabled(false);
    }
}

void Editor::setMainPhoto(const QModelIndex &index)
{
    QPushButton * const photo = ui->mainPhoto;
    Image *img = (Image *)index.data().toULongLong();

    ui->listView->selectionModel()->clear();

    if (!index.isValid() || !img) {
        photo->setIcon(QIcon(":/icons/image"));
        specimen->setMainPhotoId(0);
        return;
    }

    photo->setIconSize(photo->size());
    photo->setIcon(QIcon(*img->getScaled(photo->size())));
    specimen->setMainPhotoId(img->id());

    ui->listView->selectionModel()->select(index, QItemSelectionModel::Select);
    ui->listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
}

void Editor::emitRequestGallery()
{
    emit requestGallery(specimen);
}

void Editor::mainPhotoClicked()
{
    if (ui->listView->model()->rowCount())
        emitRequestGallery();
    else
        addPhoto();
}
