#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include "imagecache.h"

ImageCache::ImageCache(QObject *parent) :
    QObject(parent)
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

    QLinkedList<QPixmap *>::iterator i = root[id].begin();
    for (; i != root[id].end(); i++)
        if ((*i)->width() < pixmap->width())
            break;
    root[id].insert(i, pixmap);
}

QPixmap *ImageCache::insertError(const int id, const QSize &size)
{
    /* Perhaps we could share empty pixmaps but they should only appear
     * in abnormal situations so it's not really worth optimizing. */

    QPixmap *pixmap = new QPixmap(size);
    pixmap->fill();

    insert(id, pixmap);

    return pixmap;
}

QPixmap *ImageCache::getPixmap(const int id, const QSize &size)
{
    if (root.size() <= id)
        return loadSlowpath(id, size);

    /* Search the list. */
    QLinkedList<QPixmap *>::iterator i = root[id].begin();
    for (; i != root[id].end(); i++)
        if ((*i)->width() == size.width() || (*i)->height() == size.height())
            break;

    if (i != root[id].end())
        return *i;

    return loadSlowpath(id, size);
}

QPixmap *ImageCache::loadSlowpath(const int id, const QSize &size)
{
    qDebug() << "Load" << id << size;

    QSqlQuery readPhoto(QString("SELECT data FROM Images WHERE id = %1").arg(id));
    if (!readPhoto.next() || readPhoto.lastError().isValid()) {
        if (readPhoto.lastError().isValid())
            qDebug() << readPhoto.lastError().text();
        else
            qDebug() << "Image not found!";

        return insertError(id, size);
    }

    QByteArray array = readPhoto.record().value(0).toByteArray();

    QPixmap tmp, *result = new QPixmap;
    tmp.loadFromData(array, "PNG");
    *result = tmp.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    insert(id, result);

    return result;
}
