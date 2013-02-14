#ifndef IMAGE_H
#define IMAGE_H

#include <QList>
#include <QObject>
#include <QSize>

class QPixmap;

class FileLoader;
class StoreHelper;

/**
 * Image - image from database with caching of scaled versions.
 *
 * Created and maintained by ImageCache.
 *
 * This class does not read from ImagesCache table, if there is cached version
 * in persistent cache, ImageCache should read it and pass to constructor of
 * this class. If nothing is passed, class presumes cache is empty and tries to
 * fill it.
 *
 * If there is no full-sized image for the class it goes into zobie mode and
 * returns empty images.
 */

/* TODO: make isNew() dependent on id < 0. (<= 0)?
 *       perhaps we will need to introduce isNew() and isBacked().
 */
/* TODO: get rid of MainPhoto -> ImageCache should decide that -> some kind of back-call.
 *       No, actually AddNew should take care of all this MainPhoto stuff.
 */

class Image : public QObject
{
    Q_OBJECT
public:
    enum Flags {
        FullRead = 1, /* Full resolution read in, nothing more can be done. */
        Zombie = 2, /* Image is not present in database. */
        ForRemoval = 4, /* Editor marked this photo for removal. */
        NewPhoto = 8, /* Editor added this photo but didn't yet commit. */
        MainPhoto = 0x10 /* New photo which will be a main one. */
    };

    /* Size of images in ImagesCache table. */
    static const QSize CachedImageSize;

    /* Create image, read initial data from fullsize. */
    explicit Image(const int id, QObject *parent = 0);
    /* Create image with initial data from ByteArray. */
    explicit Image(const int id, const int ownerId,
                   const QByteArray &rawCache, QObject *parent = 0);
    /* Create new image from file on disk. */
    explicit Image(const QString &fileName, const int ownerId,
                   QObject *parent = 0);
    ~Image();


    /* Get image id. */
    int id() const {return _id; }
    /* Get image owner's id. */
    int ownerId() const { return _ownerId; }
    void setOwner(int id) { _ownerId = id; }

    /* Mark photo as owner's main. */
    void mainPhoto() { flags |= MainPhoto; }

    void forRemoval() { flags |= ForRemoval; }
    /* Is valid - should be saved and displayed. */
    bool isValid() const;

    /* Join all worker threads and abort database accesses. */
    void sync();
    /* Start adding itself to database. */
    void beginInsert();

    /* Get image scaled to given size. */
    QPixmap *getScaled(const QSize &size);
    /* Get image of at least given size. */
    QPixmap *getScaledGe(const QSize &);


signals:
    void changed();
    void inserted();

private slots:
    void loaderDone();
    void storerDone();

private:
    /* Can attempt to read fullsized image help in finding larger image. */
    bool canReadFull() { return !(flags & FullRead) || isZombie(); }
    bool isZombie() const { return flags & Zombie; }

    /* Insert pixmap into cache. */
    QPixmap *insert(const QByteArray &raw);
    QPixmap *insert(QPixmap *pixmap);
    /* Insert pixmap scaled to given size into cache. */
    QPixmap *insertScaled(QPixmap *pixmap, const QSize &size);
    /* Read full-scaled image, insert it and scaled version into cache. */
    QPixmap *insertFullResolution(const QSize &size);
    /* Read full-scaled image from database. */
    QByteArray readFullResolution();
    /* Try to insert scaled version into cache. */
    void preserve(QPixmap *fullsize);

    /* QSize comarison helpers. */
    bool sizeSmaller(const QSize &a, const QSize &b) const
    { return a.width() < b.width() && a.height() < b.height(); }

    int _id, _ownerId;
    unsigned flags;
    QList<QPixmap *> cache;
    FileLoader *loader;
    StoreHelper *storer;
};

#endif // IMAGE_H
