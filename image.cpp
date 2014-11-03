/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "image.h"
#include "fileloader.h"
#include "logger.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

const QSize Image::CachedImageSize(400, 400);
const int Image::Quality = 75;
const int Image::CacheQuality = 100;

Image::Image(const int id, const int ownerId, QObject *parent) :
    QObject(parent), _id(id), _ownerId(ownerId), flags(0), loader(0)
{
    cache.append(new QPixmap(":/icons/image"));
}

Image::Image(const int id, const int ownerId, const QString &fileName, QObject *parent) :
    QObject(parent), _id(id), _ownerId(ownerId), flags(NewPhoto), loader(0)
{
    loader = new FileLoader(fileName, this);
    connect(loader, SIGNAL(finished()), SLOT(loaderDone()));
    loader->start();

    cache.append(new QPixmap(":/icons/image"));
}

Image::~Image()
{
    if (loaderRunning())
        Log(Assert) << "Loader still running in image" << _id;

    if (isModified() && touchesDB())
        Log(Assert) << "Image" << _id << "removed while dirty";

    while (cache.count())
        delete cache.takeFirst();
}

void Image::loaderDone()
{
    QString filename = loader->filename();
    if (filename.endsWith(".jpg", Qt::CaseInsensitive))
        raw = loader->raw();

    fullsize = QImage(loader->fullsize());
    cached = QImage(loader->scaled());

    delete loader;
    loader = 0;

    /* Check if image was read successfully. */
    if (fullsize.isNull()) {
        Log(UserError) << "Loading file" << filename << "failed";
        flags |= ForRemoval | Failed;
        emit fileLoadFailed(_id);
        return;
    }

    while (cache.count())
        delete cache.takeFirst();

    flags |= CacheLoaded | FullRequested;
    insert(new QPixmap(QPixmap::fromImage(fullsize)));
    insert(new QPixmap(QPixmap::fromImage(cached)));

    emit changed();
}

bool Image::save(QSqlDatabase &db)
{
    if (!isModified())
        return true;

    if (loader)
        loader->wait();

    if (!touchesDB()) {
        Log(Assert) << "Photo" << _id << "is new and removed";
        return true;
    }

    if (flags & NewPhoto)
        return store(db);
    else if (flags & ForRemoval)
        return drop(db);

    return true;
}

bool Image::remove()
{
    flags |= ForRemoval;

    if (flags & NewPhoto)
        return false;

    return true;
}

bool Image::store(QSqlDatabase &db)
{
    if (raw.isEmpty()) {
        QBuffer buf(&raw);
        buf.open(QIODevice::WriteOnly);
        fullsize.save(&buf, "JPG", Quality);
    }

    QString ins("INSERT INTO ImagesIndex VALUES(%1, %2)");
    QSqlQuery insertPicture(ins.arg(_id).arg(_ownerId), db);
    if (insertPicture.lastError().isValid()) {
        Log(Error) << "Image save index" << insertPicture.lastError().text()
                   << " | " << insertPicture.lastQuery();
        return false;
    }

    insertPicture.prepare("INSERT INTO Images VALUES(:id, :image)");
    insertPicture.bindValue(":id", _id);
    insertPicture.bindValue(":image", raw);
    insertPicture.exec();
    if (insertPicture.lastError().isValid()) {
        Log(Error) << "Image save image" << insertPicture.lastError().text()
                   << " | " << insertPicture.lastQuery();
        return false;
    }

    QByteArray cacheData;
    QBuffer buf(&cacheData);
    buf.open(QIODevice::WriteOnly);
    cached.save(&buf, "JPG", CacheQuality);

    insertPicture.prepare("INSERT INTO ImagesCache VALUES(:id, :image)");
    insertPicture.bindValue(":id", _id);
    insertPicture.bindValue(":image", cacheData);
    insertPicture.exec();
    if (insertPicture.lastError().isValid()) {
        Log(Error) << "Image save cache" << insertPicture.lastError().text()
                   << " | " << insertPicture.lastQuery();
        return false;
    }

    flags ^= NewPhoto;
    return true;
}

bool Image::drop(QSqlDatabase &db)
{
    QSqlQuery dropQ(QString("DELETE FROM ImagesIndex WHERE id=%1").arg(_id), db);
    if (dropQ.lastError().isValid()) {
        Log(Error) << "Image drop index" << dropQ.lastError().text();
        return false;
    }
    dropQ.exec(QString("DELETE FROM ImagesCache WHERE id=%1").arg(_id));
    if (dropQ.lastError().isValid()) {
        Log(Error) << "Image drop cache" << dropQ.lastError().text();
        return false;
    }
    dropQ.exec(QString("DELETE FROM Images WHERE id=%1").arg(_id));
    if (dropQ.lastError().isValid()) {
        Log(Error) << "Image drop" << dropQ.lastError().text();
        return false;
    }

    flags ^= ForRemoval;
    return true;
}

QPixmap *Image::getScaled(const QSize &size)
{
    /* Image from which we can scale down to required. */
    QPixmap *doubleSize = 0;
    QList<QPixmap *>::iterator i;

    if (sizeSmaller(cache.first()->size(), size)) {
        if (canReadFull())
            requestFull();
        return cache.first();
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

    if (canReadFull())
        requestFull();
    else if (!loaderRunning() && flags & CacheLoaded)
        qDebug() << "|" << __func__ << "Image strangely undersized!";

    return insertScaled(cache.first(), size);
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

    if (canReadFull())
        requestFull();

    return cache.first();
}

void Image::setCached(const QImage &image)
{
    while (cache.size())
        delete cache.takeFirst();

    cache.insert(0, new QPixmap(QPixmap::fromImage(image)));

    flags |= CacheLoaded;

    emit changed();
}

void Image::setFullsize(const QImage &image)
{
    cache.insert(0, new QPixmap(QPixmap::fromImage(image)));
    emit changed();
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
