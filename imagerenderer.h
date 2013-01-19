#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include <QStyledItemDelegate>

class ImageRenderer : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImageRenderer(QWidget *parent = 0);

    virtual void paint(QPainter *, const QStyleOptionViewItem&,
                       const QModelIndex&) const;
signals:

public slots:

};

#endif // IMAGERENDERER_H
