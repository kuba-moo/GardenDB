/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "builtins.h"
#include "database.h"
#include "imagecache.h"
#include "logger.h"
#include "specimen.h"
#include "specimenrenderer.h"

#include <QPainter>

const int SpecimenRenderer::MinWidth = 1000;

SpecimenRenderer::SpecimenRenderer(Database *db, QObject *parent) :
    QStyledItemDelegate(parent)
{
    ic = db->imageCache();
    bi = db->builtins();
}

void SpecimenRenderer::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    const int Space = 10;
    Specimen *i = (Specimen *)index.data().toULongLong();

    if (!i)
        return;

    QStyledItemDelegate::paint(painter, option, QModelIndex());

    painter->setRenderHint(QPainter::Antialiasing);

    QRect target(option.rect);
    if (target.width() < MinWidth)
        target.setWidth(MinWidth);
    int fieldWidth;
    target.setWidth(82);
    target.setHeight(62);

    target.setTop(target.top() + 1);
    target.setLeft(target.left() + 1);
    QPixmap *mainPhoto = ic->getPixmap(i->getMainPhotoId(), QSize(82, 62));
    if (mainPhoto) {
        painter->drawPixmap(target.x() + (82-mainPhoto->width())/2,
                            target.y() + (62-mainPhoto->height())/2,
                            *mainPhoto);
    }
    target.setLeft(target.left() + 82 + Space);

    fieldWidth = 130;
    target.setWidth(fieldWidth);
    fitName(painter, target, i->getName());
    target.setLeft(target.left() + fieldWidth + Space);

    fieldWidth = 260;
    target.setWidth(fieldWidth);
    QString labels1[3] = { trUtf8("Type") + ":", trUtf8("Flowering") + ":", trUtf8("Grower") + ":" };
    QString texts1[3] = { bi->getValue("Types", i->getTypeId()),
                          bi->getValue("Flowering", i->getFloweringId()),
                          i->getGrower() };
    fitThree(painter, target, labels1, texts1);
    target.setLeft(target.left() + fieldWidth + Space);

    fieldWidth = 135;
    target.setWidth(fieldWidth);
    QString labels2[3] = { trUtf8("Flavour") + ":", trUtf8("Size") + ":", trUtf8("Frost") + ":" };
    QString texts2[3] = { bi->getValue("Flavour", i->getFlavourId()),
                          i->getSize(),
                          bi->getValue("Frost", i->getFrostId()) };
    fitThree(painter, target, labels2, texts2);
    target.setLeft(target.left() + fieldWidth + Space);

    fieldWidth = 120;
    target.setWidth(fieldWidth);
    sizeToFit(painter, target, i->getFlowers());
    target.setLeft(target.left() + fieldWidth + Space);

    target.setWidth(option.rect.width() - target.left());
    target.setTop(target.top() + 2);
    painter->drawText(target, Qt::AlignVCenter | Qt::TextWordWrap, i->getDesc());
    target.setLeft(target.left() + fieldWidth + Space);
}

QSize SpecimenRenderer::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    if (index.isValid())
        return QSize(MinWidth,64);

    return QSize(0, 0);
}

void SpecimenRenderer::sizeToFit(QPainter *painter, QRectF target, const QString &value) const
{
    QRectF usedArea = painter->boundingRect(target, Qt::AlignCenter | Qt::TextWordWrap,
                                            value);
    if (target.contains(usedArea))
        painter->drawText(target, Qt::AlignCenter | Qt::TextWordWrap, value);
    else {
        painter->save();

        QFont font = painter->font();
        font.setPointSize(font.pointSize()-2);
        painter->setFont(font);

        painter->drawText(target, Qt::AlignCenter | Qt::TextWordWrap, value);

        painter->restore();
    }
}

void SpecimenRenderer::fitName(QPainter *painter, QRect target, const QString &name) const
{
    QString bold(name), normal("");
    int bracet = name.indexOf("(");
    if (bracet != -1) {
        bold = name.left(bracet);
        normal = name.mid(bracet);
    }

    painter->save();

    QFont font(painter->font());
    font.setBold(true);
    painter->setFont(font);
    QRectF usedArea = painter->boundingRect(target, Qt::AlignCenter | Qt::TextWordWrap, bold);
    painter->drawText(target, Qt::AlignCenter | Qt::TextWordWrap, bold);

    painter->restore();

    target.setTop(usedArea.bottom() + 4);

    painter->drawText(target, Qt::AlignCenter | Qt::TextWordWrap, normal);
}

void SpecimenRenderer::fitThree(QPainter *painter, QRect target,
                                const QString l[], const QString t[]) const
{
    QRect bt(target);
    const int separation = 4, vSpace = 5;
    QFont defaultF = painter->font();
    QFont labelF = painter->font();
    labelF.setItalic(true);
    labelF.setPointSize(defaultF.pointSize()-2);
    QRectF usedArea;
    int bottom[3];
    qreal labelLength = 0;

    painter->save();

    /* First find out how long labels are. */
    for (int i = 0; i < 3; i++) {
        usedArea = painter->boundingRect(bt, l[i]);
        labelLength = qMax(labelLength, usedArea.width());
    }

    /* Then draw values and remeber where they ended (bottom). */
    bt.setTop(bt.top() + 4);
    bt.setLeft(bt.left() + labelLength + separation);
    bt.setWidth(target.width() - labelLength);

    for (int i = 0; i < 3; i++) {
        QString value = t[i].isEmpty() ? "-" : t[i];
        usedArea = painter->boundingRect(bt, Qt::AlignLeft, value);
        bottom[i] = usedArea.bottom();
        painter->drawText(bt, Qt::AlignLeft, value);
        bt.setTop(bottom[i] + vSpace);
    }

    /* Then draw labels where they should be. */
    bt = target;
    bt.setTop(target.top() + 6);
    bt.setWidth(labelLength);
    painter->setFont(labelF);

    for (int i = 0; i < 3; i++) {
        painter->drawText(bt, Qt::AlignRight, l[i]);
        bt.setTop(bottom[i] + vSpace + 2);
    }

    painter->restore();
}
