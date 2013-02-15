#ifndef SPECIMENRENDERER_H
#define SPECIMENRENDERER_H

#include <QStyledItemDelegate>

class Database;
class ImageCache;

class SpecimenRenderer : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SpecimenRenderer(Database *db, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
    
public slots:
    
private:
    ImageCache *ic;
};

#endif // SPECIMENRENDERER_H
