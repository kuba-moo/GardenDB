#ifndef SPECIMENRENDERER_H
#define SPECIMENRENDERER_H

#include <QStyledItemDelegate>

class Builtins;
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
    void sizeToFit(QPainter *painter, QRectF target, const QString &value) const;
    void fitName(QPainter *painter, QRect target, const QString &name) const;
    void fitThree(QPainter *painter, QRect target, const QString l[], const QString t[]) const;

    ImageCache *ic;
    Builtins *bi;
};

#endif // SPECIMENRENDERER_H
