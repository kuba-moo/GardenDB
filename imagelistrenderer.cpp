#include "image.h"
#include "imagelistrenderer.h"

#include <QPainter>

ImageListRenderer::ImageListRenderer(const int s, QObject *parent) :
    QStyledItemDelegate(parent), margin(5), size(s), imgSize(size-2*margin)
{
}

void ImageListRenderer::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (!index.isValid() || !index.data().toULongLong())
        return;

    Image *img = (Image *)index.data().toULongLong();

    QStyledItemDelegate::paint(painter, option, QModelIndex());

    painter->setRenderHint(QPainter::Antialiasing);

    const QPixmap &pixmap = *img->getScaled(QSize(imgSize, imgSize));
    painter->drawPixmap(option.rect.left() + (imgSize - pixmap.width())/2 + margin,
                        option.rect.top() + (imgSize - pixmap.height())/2 + margin, pixmap);
    if (option.state & QStyle::State_Selected)
        painter->drawRect(option.rect.left() + margin/2,
                          option.rect.top() + margin/2,
                          imgSize + margin, imgSize + margin);
}

QSize ImageListRenderer::sizeHint(const QStyleOptionViewItem &,
                                  const QModelIndex &) const
{
    return QSize(size, size);
}
