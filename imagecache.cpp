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
    QTime t;
    t.start();
    QSqlQuery fetchAll("SELECT * FROM ImagesIndex");
    if (fetchAll.lastError().isValid()) {
        Logger::log(Error, QString("ImageCache fetch all: %1")
                           .arg(fetchAll.lastError().text()));
        return false;
    }
    QSqlQuery fetchMain("SELECT id,main_photo FROM Species");
    if (fetchMain.lastError().isValid()) {
        Log(Error) << "ImageCache fetch all:" << fetchMain.lastError().text();
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

QPixmap *ImageCache::getPixmap(const int imageId, const QSize &size, ReadHint hint)
{
    if (images.size() <= imageId || !images[imageId]) {
        if (imageId)
            Log(Error) << "Unknown image" << imageId;
        return 0;
    }

    return images[imageId]->getScaled(size);
}

QPixmap *ImageCache::getPixmapGe(const int imageId, const QSize &size, ReadHint hint)
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
    if (species.size() < spId)
        return species[0];

    return species[spId];
}

void ImageCache::addImage(const int spId, const QString &filename)
{
    const int id = insert(new Image(nextInsertId++, spId, filename, this));

    connect(images[id], SIGNAL(fileLoadFailed(int)), SLOT(removeImage(int)));

    emit changed();
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
    if (species.size() < image->ownerId())
        species.resize(image->ownerId() * 2);
    if (imageInList(species[image->ownerId()], image->id()))
        Logger::log(Error, QString("Double on species: %1").arg(image->id()));
    species[image->ownerId()].append(image);
}

void ImageCache::insertImages(Image *image)
    {
    if (images.size() < image->id())
        images.resize(image->id() * 2);
    if (images[image->id()])
        Logger::log(Error, QString("Double on images: %1").arg(image->id()));
    images[image->id()] = image;
}

#if 0
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

    if (species.size() < spId)
        species.resize(spId * 2);

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
}
#endif
