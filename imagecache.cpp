#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include "image.h"
#include "imagecache.h"
#include "oqueries.h"

const QSize ImageCache::UsableCacheSize(200, 200);

ImageCache::ImageCache(QObject *parent) :
    QObject(parent)
{
    images.resize(128);
    species.resize(128);
    spFlags.resize(128);
}

ImageCache::~ImageCache()
{
    clear();
}

bool ImageCache::imageInList(const QList<Image *> &imgs, const int &id)
{
    QList<Image *>::const_iterator i;
    for (i = imgs.constBegin(); i < imgs.constEnd(); i++)
        if ((*i)->id() == id)
            return true;
    return false;
}

void ImageCache::clear()
{
    images.clear();

    for (int i = 0; i < species.size(); i++)
        while (species[i].size())
            delete species[i].takeFirst();
}

void ImageCache::insert(Image *image)
{
    insertSpecies(image);
    insertImages(image);
}

void ImageCache::insertSpecies(Image *image)
{
    if (species.size() < image->ownerId()) {
        species.resize(image->ownerId() * 2);
        spFlags.resize(image->ownerId() * 2);
    }
    if (imageInList(species[image->ownerId()], image->id()))
        qDebug() << "Double on secies !!!" << image->id();
    species[image->ownerId()].append(image);
}

void ImageCache::insertImages(Image *image)
    {
    if (images.size() < image->id())
        images.resize(image->id() * 2);
    if (images[image->id()])
        qDebug() << "Double on images!!!" << image->id();
    images[image->id()] = image;
}

QPixmap *ImageCache::getPixmap(const int imageId, const QSize &size, ReadHint hint)
{
    if (images.size() <= imageId || !images[imageId])
        return loadSlowpath(imageId, size, hint);

    return images[imageId]->getScaled(size);
}

QPixmap *ImageCache::getPixmapGe(const int imageId, const QSize &size, ReadHint hint)
{
    if (images.size() <= imageId || !images[imageId])
        return loadSlowpath(imageId, size, hint);

    return images[imageId]->getScaledGe(size);
}

const QList<Image *> &ImageCache::getAllImages(const int spId)
{
    if (spFlags.size() <= spId || !(spFlags[spId] & ReadAll))
        getAllSlowpath(spId);

    return species[spId];
}

void ImageCache::setImages(const unsigned spId, const QList<Image *> &valid,
                           const QList<Image *> &invalid)
{
    /* First kick off the write-backs.*/
    for (int i = 0; i < valid.count(); i++) {
        if (valid[i]->id() > 0)
            continue;

        valid[i]->setParent(this);
        connect(valid[i], SIGNAL(inserted()), SLOT(imageInserted()));
        valid[i]->beginInsert();
        insertSpecies(valid[i]);
    }

    /* Then do the deletes. */
    int removeCount = 0;
    QString remove;
    for (int i = 0; i < invalid.count(); i++) {
        invalid[i]->sync();
        remove += QString("%1 id=%2").arg(removeCount++ ? "OR" : "")
                                     .arg(invalid[i]->id());
        species[spId].removeAll(invalid[i]);
        delete invalid[i];
    }

    if (removeCount) {
        QSqlQuery removeQuery("DELETE FROM Images WHERE"+remove);
        if (removeQuery.lastError().isValid())
            qDebug() << removeQuery.lastError();
        if (removeQuery.numRowsAffected() != removeCount)
            qDebug() << "Removing failed" << removeQuery.numRowsAffected() << removeCount;
        removeQuery.exec("DELETE FROM ImagesCache WHERE"+remove);
        if (removeQuery.lastError().isValid())
            qDebug() << removeQuery.lastError();
        if (removeQuery.numRowsAffected() != removeCount)
            qDebug() << "Removing failed" << removeQuery.numRowsAffected() << removeCount;
    }
}

void ImageCache::imageInserted()
{
    Image *image = (Image *)QObject::sender();
    insertImages(image);
    emit changed();
}

Image *ImageCache::loadSingle(const int imageId, const QSize &size)
{
    qDebug() << "|" << __func__ << imageId << size;

    /* Fetch from cache if exists. */
    QSqlQuery fetch(QString("SELECT Images.sp_id,ImagesCache.data FROM Images "
                            "INNER JOIN ImagesCache ON Images.id=ImagesCache.id "
                            "WHERE Images.id=%1").arg(imageId));

    if (fetch.lastError().isValid())
        qDebug() << fetch.lastError();

    if (!fetch.next() || fetch.lastError().isValid())
        return new Image(imageId, this);

    return new Image(imageId, fetch.value(0).toInt(),
                     fetch.value(1).toByteArray(), this);
}

QPixmap *ImageCache::loadSlowpath(const int id, const QSize &size, ReadHint hint)
{
    qDebug() << "|" << __func__ << id << size << hint;

    if (hint == ReadAll)
        qDebug() << "ReadAll hint not supported!";

    if (hint == MainPhoto)
        return loadMainPhoto(id, size);

    qDebug() << "No hint slowread???";

    return loadSingle(id, size)->getScaled(size);
}

QPixmap *ImageCache::loadMainPhoto(const int imageId, const QSize &size)
{
    qDebug() << "|" << __func__ << imageId << size;

    Image *start = loadSingle(imageId, size);
    insert(start);
    QString query = "SELECT Species.id,ImagesCache.id,ImagesCache.data FROM Species "
                    "LEFT OUTER JOIN ImagesCache ON Species.main_photo=ImagesCache.id "
                    "WHERE Species.id>%1 AND Species.main_photo>1 LIMIT 5";
    QSqlQuery fetch(query.arg(start->ownerId()));

    if (fetch.lastError().isValid())
        qDebug() << fetch.lastError();

    while (fetch.next()) {
        const int spId = fetch.value(0).toInt();
        const int id = fetch.value(1).toInt();

        if (images.size() > id && images[id])
            continue;

        insert(new Image(id, spId, fetch.value(2).toByteArray(), this));
    }

    return start->getScaled(size);
}

void ImageCache::getAllSlowpath(const int spId)
{
    qDebug() << "|" << __func__ << spId;

    if (species.size() < spId) {
        species.resize(spId * 2);
        spFlags.resize(spId * 2);
    }

    QSqlQuery fetch(QString("SELECT Images.id,ImagesCache.data FROM Images "
                            "LEFT OUTER JOIN ImagesCache ON Images.id=ImagesCache.id "
                            "WHERE Images.sp_id=%1").arg(spId));

    while (fetch.next()) {
        const unsigned id = fetch.value(0).toInt();

        if (imageInList(species[spId], id))
            continue;

        Image *image;
        if (fetch.value(1).isNull())
            image = new Image(id, this);
        else
            image = new Image(id, spId, fetch.value(1).toByteArray(), this);

        insert(image);
    }

    spFlags[spId] |= ReadAll;
}
