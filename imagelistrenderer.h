#ifndef IMAGELISTRENDERER_H
#define IMAGELISTRENDERER_H

#include <QStyledItemDelegate>

class ImageCache;

class ImageListRenderer : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImageListRenderer(const int size, QObject *parent = 0);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    const int margin, size, imgSize;
};

#endif // IMAGELISTRENDERER_H
