#include "database.h"
#include "gallery.h"
#include "image.h"
#include "imagecache.h"
#include "logger.h"
#include "specimen.h"
#include "ui_gallery.h"

Gallery::Gallery(Database *db, Specimen *specimen, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Gallery)
{
    ui->setupUi(this);

    connect(ui->backToTable, SIGNAL(clicked()), SIGNAL(finished()));

    ui->title->setText(specimen->getName());
    images = db->imageCache()->getAllImages(specimen->getId());

    QList<Image *>::const_iterator i;
    for (i = images.constBegin(); i < images.constEnd(); i++) {
        QPixmap *pixmap = (*i)->getScaledGe(QSize(200, 200));
        ui->listWidget->addItem(new QListWidgetItem(QIcon(*pixmap), ""));
        connect(*i, SIGNAL(changed()), SLOT(setPicture()));
    }

    connect(ui->listWidget, SIGNAL(clicked(QModelIndex)),
            SLOT(setPicture(QModelIndex)));
    connect(ui->next, SIGNAL(clicked()), SLOT(next()));
    connect(ui->previous, SIGNAL(clicked()), SLOT(previous()));
    current = 0;
    setPicture();
}

Gallery::~Gallery()
{
    delete ui;
    ui = 0;
}

void Gallery::resizeEvent(QResizeEvent *ev)
{
    setPicture(ui->listWidget->model()->index(current, 0));

    QWidget::resizeEvent(ev);
}

void Gallery::next()
{
    if (ui->listWidget->model()->rowCount()-1 > current) {
        current++;
        setPicture();
        /* Force update for previous, cause it will not get repainted w/o selection. */
        ui->listWidget->update(ui->listWidget->model()->index(current-1, 0));
    }
}

void Gallery::previous()
{
    if (current > 0) {
        current--;
        setPicture();
        /* Force update for next, cause it will not get repainted w/o selection. */
        ui->listWidget->update(ui->listWidget->model()->index(current+1, 0));
    }
}

void Gallery::setPicture()
{
    QModelIndex index = ui->listWidget->model()->index(current, 0);
    QItemSelectionModel *selection = new QItemSelectionModel(ui->listWidget->model());
    selection->select(index, QItemSelectionModel::Select);
    //selection->setCurrentIndex(index, QItemSelectionModel::Select);
    ui->listWidget->setSelectionModel(selection);
    ui->listWidget->scrollTo(index);
    ui->listWidget->update(index);
    ui->listWidget->repaint();
    setPicture(index);
}

void Gallery::setPicture(const QModelIndex &index)
{
    if (index.isValid())
        current = index.row();
    if (current >= images.size())
        return;

    QSize size = frameSize();
    size.setWidth(size.width() - 100);
    size.setHeight(size.height() - 190);
    ui->picture->setPixmap(QPixmap(*images[current]->getScaled(size)));

    ui->next->setEnabled(true);
    ui->previous->setEnabled(true);
    if (current == 0)
        ui->previous->setEnabled(false);
    if (current == images.size()-1)
        ui->next->setEnabled(false);
}
