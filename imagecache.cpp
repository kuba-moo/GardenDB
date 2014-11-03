/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "loadercontroller.h"
#include "logger.h"
#include "image.h"
#include "imagecache.h"

#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

const QSize ImageCache::UsableCacheSize(200, 200);

ImageCache::ImageCache(QObject *parent) :
    QObject(parent)
{
}

ImageCache::~ImageCache()
{
    loaderController->quit();
    loaderController->wait();
    delete loaderController;
}

bool ImageCache::load()
{
    QElapsedTimer t;
    t.start();
    QSqlQuery fetchAll("SELECT * FROM ImagesIndex");
    if (fetchAll.lastError().isValid()) {
        Log(Error) << "ImageCache fetch all: " << fetchAll.lastError().text();
        return false;
    }
    QSqlQuery fetchMain("SELECT id,main_photo FROM Species");
    if (fetchMain.lastError().isValid()) {
        Log(Error) << "ImageCache fetch main:" << fetchMain.lastError().text();
        return false;
    }

    int max_photo = 0;
    int max_specimen = 0;
    QVector<int> photoIds;
    while (fetchMain.next()) {
        int id = fetchMain.record().value("main_photo").toInt();
        int sp_id = fetchMain.record().value("id").toInt();
        if (id)
            photoIds.push_back(id);

        max_specimen = qMax(sp_id, max_specimen);
    }

    while (fetchAll.next()) {
        int id = fetchAll.record().value("id").toInt();
        max_photo = qMax(id, max_photo);

        if (id && !photoIds.contains(id))
            photoIds.push_back(id);
    }

    images.resize(max_photo * 2);
    species.resize(max_specimen * 2);
    nextInsertId = max_photo + 1;

    fetchAll.seek(-1);
    while (fetchAll.next()) {
        int id = fetchAll.record().value("id").toInt();
        int sp_id = fetchAll.record().value("sp_id").toInt();

        if (!id)
            continue;

        images[id] = new Image(id, sp_id, this);
        species[sp_id].append(images[id]);

        connect(images[id], SIGNAL(wantFull()), SLOT(loadFull()));
    }

    fetchAll.finish();
    fetchMain.finish();

    loaderController = new LoaderController();
    loaderController->getCached(photoIds.toList());
    connect(loaderController, SIGNAL(doneCached(int,QImage*)),
            SLOT(doneCached(int,QImage*)));
    connect(loaderController, SIGNAL(doneFullsize(int,QImage*)),
            SLOT(doneFullsize(int,QImage*)));

    Log(Debug) << "Database has" << photoIds.size() << "images";
    Log(Performance) << "Loading ImageCache took" << t.elapsed() << "ms";
    return true;
}

bool ImageCache::isModified()
{
    if (removed.size())
        return true;

    for (int i = 0; i < images.size(); i++)
        if (images[i] && images[i]->isModified())
            return true;

    return false;
}

int ImageCache::countModified()
{
    int count = removed.size();

    for (int i = 0; i < images.size(); i++)
        if (images[i] && images[i]->isModified())
            count++;

    return count;
}

bool ImageCache::save(QSqlDatabase &db)
{
    while (removed.size()) {
        Image *img = removed.takeFirst();
        if (!img->save(db))
            return false;
        emit oneSaved();
        delete img;
    }

    for (int i = 0; i < images.size(); i++)
        if (images[i] && images[i]->isModified()) {
            if (!images[i]->save(db))
                return false;

            emit oneSaved();
        }

    return true;
}

Image *ImageCache::getImage(const int id)
{
    if (images.size() <= id || !images[id]) {
        if (id)
            Log(Error) << "Unknown image" << id;
        return 0;
    }

    return images[id];
}

QPixmap *ImageCache::getPixmap(const int imageId, const QSize &size)
{
    if (images.size() <= imageId || !images[imageId]) {
        if (imageId)
            Log(Error) << "Unknown image" << imageId;
        return 0;
    }

    return images[imageId]->getScaled(size);
}

QPixmap *ImageCache::getPixmapGe(const int imageId, const QSize &size)
{
    if (images.size() <= imageId || !images[imageId]) {
        if (imageId)
            Log(Error) << "Unknown image" << imageId;
        return 0;
    }


    return images[imageId]->getScaledGe(size);
}

const QList<Image *> &ImageCache::getAllImages(const int spId)
{
    if (species.size() <= spId)
        species.resize(spId * 2 + 1);

    return species[spId];
}

Image *ImageCache::addImage(const int spId, const QString &filename)
{
    const int id = insert(new Image(nextInsertId++, spId, filename, this));

    connect(images[id], SIGNAL(fileLoadFailed(int)), SLOT(removeImage(int)));
    connect(images[id], SIGNAL(wantFull()), SLOT(loadFull()));

    emit changed();

    return images[id];
}

void ImageCache::removeImage(const int id)
{
    species[images[id]->ownerId()].removeOne(images[id]);

    if (images[id]->remove())
        removed.append(images[id]);
    else
        delete images[id];

    images[id] = 0;

    emit changed();
}

void ImageCache::loadFull()
{
    Image *img = (Image *)QObject::sender();
    loaderController->getFullsize(img->id());
}

void ImageCache::doneCached(int id, QImage *img)
{
    if (images[id])
        images[id]->setCached(*img);

    delete img;
}

void ImageCache::doneFullsize(int id, QImage *img)
{
    if (images[id])
        images[id]->setFullsize(*img);

    delete img;
}

bool ImageCache::imageInList(const QList<Image *> &imgs, const int &id) const
{
    QList<Image *>::const_iterator i;
    for (i = imgs.constBegin(); i < imgs.constEnd(); i++)
        if ((*i)->id() == id)
            return true;
    return false;
}

int ImageCache::insert(Image *image)
{
    insertSpecies(image);
    insertImages(image);

    emit changed();

    return image->id();
}

void ImageCache::insertSpecies(Image *image)
{
    if (species.size() <= image->ownerId())
        species.resize(image->ownerId() * 2);
    if (imageInList(species[image->ownerId()], image->id()))
        Log(Error) << "Double on species: " << image->id();
    species[image->ownerId()].append(image);
}

void ImageCache::insertImages(Image *image)
    {
    if (images.size() <= image->id())
        images.resize(image->id() * 2);
    if (images[image->id()])
        Log(Error) << "Double on images: " << image->id();
    images[image->id()] = image;
}
