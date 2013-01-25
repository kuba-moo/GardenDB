#ifndef IMAGE_H
#define IMAGE_H

#include <QList>
#include <QObject>
#include <QSize>

class QPixmap;

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

class Image : public QObject
{
    Q_OBJECT
public:
    /* Create image, read initial data from fullsize. */
    explicit Image(const int id, QObject *parent = 0);
    /* Create image with initial data from ByteArray. */
    explicit Image(const int id, const int ownerId,
                   const QByteArray &rawCache, QObject *parent = 0);
    ~Image();

    /* Get image id. */
    int id() const {return _id; }
    /* Get image owner's id. */
    int ownerId() const { return _ownerId; }

    /* Get image scaled to given size. */
    QPixmap *getScaled(const QSize &size);
    /* Get image of at least given size. */
    QPixmap *getScaledGe(const QSize &);

private:
    /* Size of images in ImagesCache table. */
    static const QSize CachedImageSize;

    /* Full-size image read from database. */
    bool isFullRead() { return fullRead && !zombieMode; }

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
    bool fullRead; /* Full resolution read in, nothing more can be done. */
    bool zombieMode; /* Image is not present in database. */
    QList<QPixmap *> cache;
};

#endif // IMAGE_H
