#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include "imagecache.h"
#include "oqueries.h"

ImageCache::ImageCache(QObject *parent) :
    QObject(parent),
    CachedImageSize(400, 400),
    UsableCacheSize(200, 200),
    MaxCost(400*1000*1000)
{
    root.resize(128);
}

void ImageCache::clear()
{
    for (int i = 0; i < root.size(); i++)
        while (root[i].size())
            delete root[i].takeFirst();
}

void ImageCache::insert(const int id, QPixmap *pixmap)
{
    if (root.size() <= id)
        root.resize(id*2);

    reserveSpace(pixmap);

    QLinkedList<QPixmap *>::iterator i = root[id].begin();
    for (; i != root[id].end(); i++)
        if ((*i)->width() < pixmap->width())
            break;
    root[id].insert(i, pixmap);
}

void ImageCache::reserveSpace(QPixmap *pixmap)
{
    cost += 1000 + pixmap->width()*pixmap->height()*4;

    if (cost > MaxCost) {
        clear();
        cost = 1000 + pixmap->width()*pixmap->height()*4;
    }
}

void ImageCache::preserve(const int id, QPixmap *fullsize)
{
    qDebug() << "Preserve" << id;

    /* If image is to small there is no use in scaling it down. */
    if (fullsize->size().width() < CachedImageSize.width() ||
        fullsize->size().height() < CachedImageSize.height())
        return;

    QSqlQuery insertPicture;
    insertPicture.prepare("INSERT INTO ImagesCache VALUES(:id, :image)");

    QByteArray array;
    QBuffer buf(&array);
    QPixmap pixmap = fullsize->scaled(CachedImageSize,
                                      Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation);
    buf.open(QIODevice::WriteOnly);
    pixmap.save(&buf, "PNG");

    insertPicture.bindValue(":id", id);
    insertPicture.bindValue(":image", array);
    insertPicture.exec();

    if (insertPicture.lastError().isValid())
        qDebug() << insertPicture.lastError();
}

QPixmap *ImageCache::insertError(const int id, const QSize &size)
{
    /* Perhaps we could share empty pixmaps but they should only appear
     * in abnormal situations so it's not really worth optimizing. */
    /* Create and fill with zeroes. */
    QPixmap *pixmap = new QPixmap(size);
    pixmap->fill();

    insert(id, pixmap);

    return pixmap;
}

QPixmap *ImageCache::getPixmap(const int id, const QSize &size)
{
    if (id == 1)
        return insertError(id, size);

    if (root.size() <= id)
        return loadSlowpath(id, size);

    /* Search the list. */
    QLinkedList<QPixmap *>::iterator i = root[id].begin();
    for (; i != root[id].end(); i++)
        if (((*i)->width() == size.width() && (*i)->height() <= size.height()) ||
            ((*i)->width() <= size.width() && (*i)->height() == size.height()))
            break;

    if (i != root[id].end())
        return *i;

    return loadSlowpath(id, size);
}

QPixmap *ImageCache::getPixmapGe(const int id, const QSize &size)
{
    QPixmap *ret = NULL;

    if (id == 1)
        return insertError(id, size);

    if (root.size() <= id)
        return loadSlowpath(id, size);

    /* Search the list - remember smallest bigger. */
    QLinkedList<QPixmap *>::iterator i = root[id].begin();
    for (; i != root[id].end(); i++)
        if ((*i)->width() >= size.width() || (*i)->height() >= size.height())
            ret = *i;

    if (ret)
        return ret;

    return loadSlowpath(id, size);
}

QPixmap *ImageCache::loadSlowpath(const int id, const QSize &size)
{
    QByteArray binaryPhoto;

    qDebug() << "Load" << id << size;

    /* First try to dig it out of the persistent cache. */
    QSqlQuery searchCache(QString("SELECT data FROM ImagesCache WHERE id = %1").arg(id));
    if (searchCache.lastError().isValid())
            qDebug() << searchCache.lastError().text();

    if (searchCache.next() && canUsePCache(size))
            binaryPhoto = searchCache.record().value(0).toByteArray();
    else { /* Miss, we need to go deeper... */
        qDebug() << "Miss" << id;

        QSqlQuery readPhoto(QString("SELECT data FROM Images WHERE id = %1").arg(id));
        if (!readPhoto.next() || readPhoto.lastError().isValid()) {
            if (readPhoto.lastError().isValid())
                qDebug() << readPhoto.lastError().text();
            else
                qDebug() << "Image not found!";

            return insertError(id, size);
        }

        binaryPhoto = readPhoto.record().value(0).toByteArray();
    }

    QPixmap tmp, *result = new QPixmap;
    tmp.loadFromData(binaryPhoto, "PNG");
    *result = tmp.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!searchCache.isValid())
        preserve(id, &tmp);
    insert(id, result);

    return result;
}
