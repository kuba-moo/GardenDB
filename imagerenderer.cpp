#include "imagerenderer.h"
#include "imagecache.h"

#include <QDebug>
#include <QLabel>
#include <QFile>
#include <QApplication>

ImageRenderer::ImageRenderer(QWidget *parent) :
    QStyledItemDelegate(parent)
{
}

void ImageRenderer::paint(QPainter *painter,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    QPixmap *pixmap = ImageCache::getInstance().getPixmap(index.data().toInt(),
                                                          option.rect.size());

    QApplication::style()->drawItemPixmap(painter, option.rect, 0, *pixmap);
}
