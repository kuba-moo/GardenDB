#include "image.h"
#include "fileloader.h"
#include "storehelper.h"

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>
#include <QVariant>

const QSize Image::CachedImageSize(400, 400);

Image::Image(const int id, QObject *parent) :
    QObject(parent), _id(id), _ownerId(0), flags(0), loader(0), storer(0)
{
    if (id == 1) {
        flags |= Zombie;
        return;
    }

    const QByteArray raw = readFullResolution();
    /* Failed to read? */
    if (isZombie())
        return;
    preserve(insert(raw));
}

Image::Image(const int id, const int ownerId,
             const QByteArray &rawCache, QObject *parent) :
    QObject(parent), _id(id), _ownerId(ownerId), flags(0), loader(0), storer(0)
{
    insert(rawCache);
}

Image::Image(const QString &fileName, const int ownerId, QObject *parent) :
    QObject(parent), _id(-1), _ownerId(ownerId), flags(0), storer(0)
{
    loader = new FileLoader(fileName, this);
    connect(loader, SIGNAL(finished()), SLOT(loaderDone()));
    loader->start();
    cache.append(new QPixmap(":/icons/image"));
}

Image::~Image()
{
    sync();

    while (cache.count())
        delete cache.takeFirst();
}

void Image::sync()
{
    if (loader) {
        loader->blockSignals(true);
        loader->wait();
        delete loader;
    }

    if (storer) {
        storer->blockSignals(true);
        storer->wait();
        delete storer;
    }
}

void Image::beginInsert()
{
    if (loader)
        loader->wait();
    storer = new StoreHelper(cache.first()->toImage(), this);
    connect(storer, SIGNAL(finished()), SLOT(storerDone()));
    storer->start();
}


void Image::loaderDone()
{
    QPixmap *fullsize = new QPixmap(QPixmap::fromImage(loader->fullsize()));
    QPixmap *scaled = new QPixmap(QPixmap::fromImage(loader->scaled()));
    delete cache.takeFirst();
    delete loader;
    loader = 0;

    /* Check if image was read successfully. */
    if (fullsize->isNull()) {
        flags |= Zombie;
        delete fullsize;
        delete scaled;
        return;
    }

    flags |= FullRead;
    insert(fullsize);
    insert(scaled);

    emit changed();
}

void Image::storerDone()
{
    QSqlQuery insertPicture;
    insertPicture.prepare("INSERT INTO Images VALUES(NULL, :image, :id)");

    insertPicture.bindValue(":id", ownerId());
    insertPicture.bindValue(":image", storer->fullsize());
    insertPicture.exec();
    if (insertPicture.lastError().isValid())
        qDebug() << insertPicture.lastError();
    _id = insertPicture.lastInsertId().toInt();

    insertPicture.prepare("INSERT INTO ImagesCache VALUES(:id, :image)");

    insertPicture.bindValue(":id", id());
    insertPicture.bindValue(":image", storer->cache());
    insertPicture.exec();
    if (insertPicture.lastError().isValid())
        qDebug() << insertPicture.lastError();

    if (flags & MainPhoto) {
        QSqlQuery setMain(QString("UPDATE Species SET main_photo=%1 WHERE id=%2")
                          .arg(id()).arg(ownerId()));
        if (setMain.lastError().isValid())
            qDebug() << setMain.lastError();
    }

    emit inserted();
}

bool Image::isValid() const
{
    if (flags & ForRemoval)
        return false;

    return !(isZombie() && (flags & NewPhoto));
}

QPixmap *Image::getScaled(const QSize &size)
{
    /* Image from which we can scale down to required. */
    QPixmap *doubleSize = 0;
    QList<QPixmap *>::iterator i;

    if (loader)
        return cache.first();

    /* Cache can be empty in zombies. */
    if (cache.empty() || sizeSmaller(cache.first()->size(), size)) {
        if (canReadFull())
            return insertFullResolution(size);
        else {
            qDebug() << "|" << __func__ << "Image undersized!";
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

    if (canReadFull())
        return insertFullResolution(size);

    qDebug() << "|" << __func__ << "Image strangely undersized!";
    return cache.first();
}

QPixmap *Image::getScaledGe(const QSize &size)
{
    if (loader)
        return cache.first();

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
    if (isZombie()) {
        QPixmap *empty = new QPixmap(size);
        empty->fill();
        return insert(empty);
    }

    QPixmap *full = insert(readFullResolution());
    return insertScaled(full, size);
}

QByteArray Image::readFullResolution()
{
    qDebug() << "|" << __func__ << _id << cache.size()
             << (cache.size() ? cache.first()->size() : QSize(-2, -2));

    flags |= FullRead;

    QSqlQuery read(QString("SELECT data,sp_id FROM Images WHERE id = %1").arg(_id));

    if (read.lastError().isValid())
        qDebug() << read.lastError();
    if (!read.next() || read.lastError().isValid()) {
        qDebug() << "Image" << _id << "going to zombieMode";
        flags |= Zombie;
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
