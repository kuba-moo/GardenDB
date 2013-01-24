#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include <QStyledItemDelegate>

class ImageCache;

class ImageRenderer : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImageRenderer(ImageCache *imageCache, QWidget *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionViewItem&,
                       const QModelIndex&) const;

private:
    ImageCache *ic;
};

#endif // IMAGERENDERER_H
