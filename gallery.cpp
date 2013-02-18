#include "database.h"
#include "gallery.h"
#include "image.h"
#include "imagecache.h"
#include "imagelistmodel.h"
#include "imagelistrenderer.h"
#include "logger.h"
#include "specimen.h"
#include "ui_gallery.h"

#include <QScrollBar>
#include <QTimer>

Gallery::Gallery(Database *db, Specimen *specimen, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Gallery)
{
    ui->setupUi(this);

    connect(ui->backToTable, SIGNAL(clicked()), SIGNAL(finished()));

    ui->title->setText(specimen->getName());
    ImageListModel *iml = new ImageListModel(db->imageCache(),
                                             specimen->getId(), this);
    ui->listView->setItemDelegate(new ImageListRenderer(124, this));
    ui->listView->setModel(iml);
    connect(iml, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(imageChanged(QModelIndex,QModelIndex)));

    connect(ui->listView, SIGNAL(clicked(QModelIndex)),
            SLOT(setPicture(QModelIndex)));
    connect(ui->next, SIGNAL(clicked()), SLOT(next()));
    connect(ui->previous, SIGNAL(clicked()), SLOT(previous()));
    current = 0;
    cutWidth = 220;

    QTimer::singleShot(15, this, SLOT(lateInit()));
}

Gallery::~Gallery()
{
    delete ui;
    ui = 0;
}

void Gallery::resizeEvent(QResizeEvent *ev)
{
    setPicture(ui->listView->model()->index(current, 0));

    QWidget::resizeEvent(ev);
}

void Gallery::next()
{
    if (ui->listView->model()->rowCount()-1 > current) {
        current++;
        setPicture();
        /* Force update for previous, cause it will not get repainted w/o selection. */
        ui->listView->update(ui->listView->model()->index(current-1, 0));
    }
}

void Gallery::previous()
{
    if (current > 0) {
        current--;
        setPicture();
        /* Force update for next, cause it will not get repainted w/o selection. */
        ui->listView->update(ui->listView->model()->index(current+1, 0));
    }
}

void Gallery::setPicture()
{
    QModelIndex index = ui->listView->model()->index(current, 0);
    QItemSelectionModel *selection = new QItemSelectionModel(ui->listView->model());
    selection->select(index, QItemSelectionModel::Select);
    selection->setCurrentIndex(index, QItemSelectionModel::Select);
    ui->listView->setSelectionModel(selection);
    ui->listView->scrollTo(index);
    ui->listView->update(index);
    setPicture(index);
}

void Gallery::setPicture(const QModelIndex &index)
{
    if (index.isValid())
        current = index.row();
    if (current >= ui->listView->model()->rowCount())
        return;

    /* Calculate maximum size of pixmap in current window. */
    QSize size = frameSize();
    size.setWidth(size.width() - 160);
    size.setHeight(size.height() - cutWidth);
    Image *img = (Image *)ui->listView->model()->index(current, 0).data().toULongLong();
    ui->picture->setPixmap(QPixmap(*img->getScaled(size)));

    ui->next->setEnabled(true);
    ui->previous->setEnabled(true);
    if (current == 0)
        ui->previous->setEnabled(false);
    if (current == ui->listView->model()->rowCount()-1)
        ui->next->setEnabled(false);
}

void Gallery::imageChanged(const QModelIndex &index,const QModelIndex &)
{
    QModelIndex current = ui->listView->selectionModel()->currentIndex();

    if (current.row() == index.row())
        setPicture(current);
}

void Gallery::lateInit()
{
    if (ui->listView->horizontalScrollBar()->isVisible()) {
        cutWidth += 18;
        ui->listView->setMaximumHeight(ui->listView->height() + 18);
    }

    setPicture();
}
