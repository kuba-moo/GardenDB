/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
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
