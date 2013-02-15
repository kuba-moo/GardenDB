#include "database.h"
#include "imagecache.h"
#include "logger.h"
#include "specimen.h"
#include "specimenrenderer.h"

#include <QPainter>

SpecimenRenderer::SpecimenRenderer(Database *db, QObject *parent) :
    QStyledItemDelegate(parent)
{
    ic = db->imageCache();
}

void SpecimenRenderer::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    Specimen *i = (Specimen *)index.data().toULongLong();
    int top = option.rect.top();
    int left = option.rect.left();

    if (!i)
        return;

    if (!index.isValid())
        Log(Error) << "Invalid!";

    QRect r(option.rect);
    r.setLeft(65);
    r.setWidth(10);

    QPixmap *mainPhoto = ic->getPixmap(i->getMainPhotoId(), QSize(54, 54));
    if (mainPhoto)
        painter->drawPixmap(left + 3, top + 3, mainPhoto->width(), mainPhoto->height(), *mainPhoto);
    painter->drawText(r, Qt::AlignCenter, QString::number(i->getNo()));
    r.setLeft(80);
    r.setWidth(80);
    painter->drawText(r, Qt::AlignCenter, i->getName());

    if (option.state & QStyle::State_Selected)
        painter->drawEllipse(5, option.rect.top() + 5, 50, 50);
}

QSize SpecimenRenderer::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        if (option.state & QStyle::State_Selected)
            return QSize(150, 70);
        return QSize(150,60);
    }

    return QSize(0, 0);
}
