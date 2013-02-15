#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <QList>
#include <QObject>
#include <QSize>

class QPixmap;

class FileLoader;

/**
 * @brief Image from database with caching of scaled versions.
 *
 * Created and maintained by ImageCache.
 *
 * This class does not read directly from Database, ImageLoader should do that
 * when requested and pass result here.
 */

class Image : public QObject
{
    Q_OBJECT
public:
    enum Flags {
        ForRemoval = 0x01, /* Editor marked this photo for removal. */
        NewPhoto = 0x02, /* Photo not yet stored in database. */
        Failed = 0x04, /* Image failed to load. */
        CacheLoaded = 0x08, /* Cached version was loaded. */
        FullRequested = 0x10 /* Full resolution was requested. */
    };

    /* Size of images in ImagesCache table. */
    static const QSize CachedImageSize;
    static const int Quality;
    static const int CacheQuality;

    /* Create image, cache will be read by ImageLoader. */
    explicit Image(const int id, const int ownerId, QObject *parent = 0);
    /* Create new image from file on disk. */
    explicit Image(const int id, const int ownerId, const QString &fileName,
                   QObject *parent = 0);

    ~Image();

    /* Get image id. */
    int id() const {return _id; }
    /* Get image owner's id. */
    int ownerId() const { return _ownerId; }

    /* True if image needs to be saved. */
    bool isModified() const { return flags & (NewPhoto | ForRemoval); }
    /* Save changes to the database, return true if photo is still needed. */
    bool save();
    /* Remove photo, return true if photo needs to be kept until save time. */
    bool remove();

    /* Get image scaled to given size. */
    QPixmap *getScaled(const QSize &size);
    /* Get image of at least given size. */
    QPixmap *getScaledGe(const QSize &);

    /* Pass loaded cached image. */
    void setCached(const QImage &image);
    /* Pass loaded fullsized image. */
    void setFullsize(const QImage &image);

signals:
    /* Loading image from file failed. */
    void fileLoadFailed(int);
    void changed();
    /* Image needs full-resolution version. */
    void wantFull();

private slots:
    void loaderDone();

private:
    /* Write image to database. [non-GUI thread] */
    bool store();
    /* Remove image from database. [non-GUI thread] */
    bool drop();

    /* Image is or will be stored in database. */
    bool touchesDB() const
    { return (flags & (NewPhoto | ForRemoval)) != (NewPhoto | ForRemoval); }
    /* Loader is running. */
    bool loaderRunning() const { return loader; }

    /* Can attempt to read fullsized image. */
    bool canReadFull() { return !(flags & FullRequested) && (flags & CacheLoaded); }
    /* Send request to loader asking for full-resolution picture.*/
    void requestFull() { flags |= FullRequested; emit wantFull(); }

    /* Insert pixmap into cache. */
    QPixmap *insert(QPixmap *pixmap);
    /* Insert pixmap scaled to given size into cache. */
    QPixmap *insertScaled(QPixmap *pixmap, const QSize &size);

    /* QSize comarison helpers. */
    bool sizeSmaller(const QSize &a, const QSize &b) const
    { return a.width() < b.width() && a.height() < b.height(); }

    int _id, _ownerId;
    unsigned flags;
    QList<QPixmap *> cache;
    FileLoader *loader;

    /* If file was loaded from disk, this holds pixmaps to be saved. */
    QByteArray raw;
    QImage fullsize, cached;


#if 0 /* Compatibility*/
public:
#if 0
    /* Create image, read initial data from fullsize. */
    explicit Image(const int id, QObject *parent = 0);
    /* Create image with initial data from ByteArray. */
    explicit Image(const int id, const int ownerId,
                   const QByteArray &rawCache, QObject *parent = 0);
    /* Create new image from file on disk. */
    explicit Image(const QString &fileName, const int ownerId,
                   QObject *parent = 0);

    /* TODO: remove Image::setOwner */
    void setOwner(int id) { _ownerId = id; }

    /* Mark photo as owner's main. */
    void mainPhoto() { flags |= MainPhoto; }

    void forRemoval() { flags |= ForRemoval; }
#endif
    /* Is valid - should be saved and displayed. */
    bool isValid() const { return true; }
#endif
};

#endif // IMAGE_H
