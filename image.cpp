#include "image.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

const QSize Image::CachedImageSize(400, 400);

Image::Image(const int id, QObject *parent) :
    QObject(parent), _id(id), _ownerId(0), fullRead(false), zombieMode(id == 1)
{
    if (zombieMode)
        return;

    const QByteArray raw = readFullResolution();
    /* Failed to read? */
    if (zombieMode)
        return;
    preserve(insert(raw));
}

Image::Image(const int id, const int ownerId,
             const QByteArray &rawCache, QObject *parent) :
    QObject(parent), _id(id), _ownerId(ownerId), fullRead(false), zombieMode(id == 1)
{
    insert(rawCache);
}

Image::~Image()
{
    while (cache.count())
        delete cache.takeFirst();
}

QPixmap *Image::getScaled(const QSize &size)
{
    /* Image from which we can scale down to required. */
    QPixmap *doubleSize = 0;
    QList<QPixmap *>::iterator i;

    /* Cache can be empty in zombies. */
    if (cache.empty() || sizeSmaller(cache.first()->size(), size)) {
        if (!isFullRead())
            return insertFullResolution(size);
        else {
            qDebug() << __PRETTY_FUNCTION__ << "Image undersized!";
            return cache.first();
        }
    }

    /* Search the list. */
    for (i = cache.begin(); i != cache.end(); i++) {
        /* Search until hit smaller than required. */
        if ((*i)->width() <= size.width() && (*i)->height() <= size.height())
            break;
        /* Remeber smallest, but at least twice as large as required. */
        if ((*i)->width() >= size.width()*2 && (*i)->height() >= size.height()*2)
            doubleSize = *i;
    }

    if (i != cache.end() &&
        (((*i)->width() == size.width() && (*i)->height() <= size.height()) ||
        ((*i)->width() <= size.width() && (*i)->height() == size.height())))
        return *i;

    if (doubleSize)
        return insertScaled(doubleSize, size);

    if (!isFullRead())
        return insertFullResolution(size);

    qDebug() << __PRETTY_FUNCTION__ << "Image strangely undersized!";
    return cache.first();
}

QPixmap *Image::getScaledGe(const QSize &size)
{
    QList<QPixmap *>::iterator i;
    /* Break on smallest bigger. */
    for (i = cache.begin(); i != cache.end(); i++)
        if (sizeSmaller((*i)->size(), size))
            break;
    if (i != cache.begin())
        return *--i;

    return insertFullResolution(size);
}

QPixmap *Image::insert(const QByteArray &raw)
{
    QPixmap *pixmap = new QPixmap;
    pixmap->loadFromData(raw, "PNG");

    return insert(pixmap);
}

QPixmap *Image::insert(QPixmap *pixmap)
{
    QList<QPixmap *>::iterator i;
    for (i = cache.begin(); i != cache.end(); i++)
        if (sizeSmaller((*i)->size(), pixmap->size()))
            break;
    cache.insert(i, pixmap);

    return pixmap;
}

QPixmap *Image::insertScaled(QPixmap *pixmap, const QSize &size)
{
    return insert(new QPixmap(pixmap->scaled(size, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation)));
}

QPixmap *Image::insertFullResolution(const QSize &size)
{
    if (zombieMode) {
        QPixmap *empty = new QPixmap(size);
        empty->fill();
        return insert(empty);
    }

    QPixmap *full = insert(readFullResolution());
    return insertScaled(full, size);
}

QByteArray Image::readFullResolution()
{
    qDebug() << __PRETTY_FUNCTION__ << _id << cache.size()
             << (cache.size() ? cache.first()->size() : QSize());

    fullRead = true;

    QSqlQuery read(QString("SELECT data,sp_id FROM Images WHERE id = %1").arg(_id));

    if (read.lastError().isValid())
        qDebug() << read.lastError().text();
    if (!read.next() || read.lastError().isValid()) {
        qDebug() << "Image" << _id << "going to zombieMode";
        zombieMode = true;
        return QByteArray();
    }

    _ownerId = read.value(1).toInt();

    return read.value(0).toByteArray();
}

void Image::preserve(QPixmap *fullsize)
{
    QSqlQuery insertPicture;
    insertPicture.prepare("INSERT INTO ImagesCache VALUES(:id, :image)");

    QPixmap cached;
    /* If image is to small there is no use in scaling it down. */
    if (fullsize->size().width() < CachedImageSize.width() ||
        fullsize->size().height() < CachedImageSize.height())
        cached = QPixmap(*fullsize);
    else
        cached = fullsize->scaled(CachedImageSize,
                                  Qt::KeepAspectRatioByExpanding,
                                  Qt::SmoothTransformation);
    QByteArray array;
    QBuffer buf(&array);
    buf.open(QIODevice::WriteOnly);
    cached.save(&buf, "PNG");

    insertPicture.bindValue(":id", _id);
    insertPicture.bindValue(":image", array);
    insertPicture.exec();

    if (insertPicture.lastError().isValid())
        qDebug() << insertPicture.lastError();
}
