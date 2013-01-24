#include "imagerenderer.h"
#include "imagecache.h"

#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QApplication>

ImageRenderer::ImageRenderer(ImageCache *imageCache, QWidget *parent) :
    QStyledItemDelegate(parent)
{
    ic = imageCache;
}

void ImageRenderer::paint(QPainter *painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    QPixmap *pixmap = ic->getPixmap(index.data().toInt(), option.rect.size());

    QApplication::style()->drawItemPixmap(painter, option.rect, 0, *pixmap);
}
