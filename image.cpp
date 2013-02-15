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
#include <QTime>
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
        Logger::log(UserError, "Loading file " + filename + " failed");
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

bool Image::save()
{
    if (!isModified())
        return true;

    if (loader)
        loader->wait();

    if (!touchesDB()) {
        Log(Assert) << "Photo" << _id << "is new and removed";
        return false;
    }

    if (flags & NewPhoto)
        return store();
    else if (flags & ForRemoval)
        return drop();

    return true;
}

bool Image::remove()
{
    flags |= ForRemoval;

    if (flags & NewPhoto)
        return false;

    return true;
}

bool Image::store()
{
    if (raw.isEmpty()) {
        QBuffer buf(&raw);
        buf.open(QIODevice::WriteOnly);
        fullsize.save(&buf, "JPG", Quality);
    }

    QSqlDatabase::database().transaction();

    QString ins("INSERT INTO ImagesIndex VALUES(%1, %2)");
    QSqlQuery insertPicture(ins.arg(_id).arg(_ownerId));
    if (insertPicture.lastError().isValid()) {
        Logger::log(Error, "Image save index: " +
                           insertPicture.lastError().text());
        QSqlDatabase::database().rollback();
        return true;
    }

    insertPicture.prepare("INSERT INTO Images VALUES(:id, :image)");
    insertPicture.bindValue(":id", _id);
    insertPicture.bindValue(":image", raw);
    insertPicture.exec();
    if (insertPicture.lastError().isValid()) {
        Logger::log(Error, "Image save image: " +
                           insertPicture.lastError().text());
        QSqlDatabase::database().rollback();
        return true;
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
        Logger::log(Error, "Image save imageCache: " +
                           insertPicture.lastError().text());
        QSqlDatabase::database().rollback();
        return true;
    }

    if (!QSqlDatabase::database().commit()) {
        Logger::log(Error, "Image save commit: " +
                           insertPicture.lastError().text());
        return true;
    }

    flags ^= NewPhoto;
    return true;
}

bool Image::drop()
{
    /* TODO: see if this removes Cache and Image. */
    QSqlQuery dropQ(QString("DELETE FROM ImagesIndex WHERE id=%1").arg(_id));
    if (dropQ.lastError().isValid()) {
        Logger::log(Error, "Image drop: " + dropQ.lastError().text());
        return false;
    }

    return false;
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
}

void Image::setFullsize(const QImage &image)
{
    cache.insert(0, new QPixmap(QPixmap::fromImage(image)));
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





/*
Image::Image(const int id, QObject *parent) :
    QObject(parent), _id(id), _ownerId(0), flags(0), loader(0), storer(0)
{
    if (id == 1) {
        flags |= Zombie;
        return;
    }

    const QByteArray raw = readFullResolution();

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


bool Image::isValid() const
{
    if (flags & ForRemoval)
        return false;

    return !(isZombie() && (flags & NewPhoto));
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
*/
