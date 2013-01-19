#include "imagerenderer.h"

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
    QByteArray data = index.data().toByteArray();
    if (data.count() < 128)
        return;

    QPixmap pixmap;
    pixmap.loadFromData(data, "PNG");

    QApplication::style()->drawItemPixmap(painter, option.rect,
                                          0, pixmap.scaled(option.rect.size(),
                                                           Qt::KeepAspectRatio,
                                                           Qt::FastTransformation));
}
