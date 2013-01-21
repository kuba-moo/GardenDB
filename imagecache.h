#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QLinkedList>
#include <QObject>
#include <QVector>

#include "oqueries.h"

class QPixmap;
class QWidget;

/** ImageCache
 *
 * It's not a cache acutally, more like a accessor. You don't insert anything
 * into it and if it doesn't have the object stored it will fetch it from the
 * database.
 *
 * Database:
 * Pictures are stored in database in "full" and in "pre-scaled" form. They
 * are scaled down to size CachedImageSize which is used to generate pictures
 * smaller than UsableCacheSize. Pictures are inserted to "pre-scaled" cache
 * on first use for backwards compatibility (old dbs don't have that table.)
 *
 * In Image table, first image (id=1) is always empty - null or maybe even
 * doesn't exist after cascade delete.
 *
 * All the elements are scaled with Qt::KeepAspectRatio.
 */

/* TODO: regenerate images from those already in cache. */

class ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache& getInstance()
    {
        static ImageCache instance;

        return instance;
    }

    /* Clear cache. */
    void clear();

    /* Read form cache pixmap of exactly given size. */
    QPixmap *getPixmap(const int, const QSize &);
    /* Get pixmap greater or equal to given size. */
    QPixmap *getPixmapGe(const int, const QSize &);

private:
    /* Size of images in ImagesCache table. */
    const QSize CachedImageSize;
    /* Max size of image that can be generated from cached images. */
    const QSize UsableCacheSize;


    explicit ImageCache(QObject *parent = 0);
    ImageCache(ImageCache const&);
    void operator=(ImageCache const&);

    /* Can persistent cache be used to generate image. */
    bool canUsePCache(const QSize &size) const
    {
        return size.width() <= UsableCacheSize.width() &&
               size.height() <= UsableCacheSize.height();
    }

    /* Insert pixmap into cache. */
    void insert(const int, QPixmap *);
    /* Update cost and kick out oldest images if needed. */
    void reserveSpace(QPixmap *);
    /* Save image into ImagesCache. */
    void preserve(const int, QPixmap *);

    /* Insert empty pixmap into cache. */
    QPixmap *insertError(const int, const QSize &);
    /* Load from database, resize and insert. */
    QPixmap *loadSlowpath(const int, const QSize &);


    QVector<QLinkedList<QPixmap *> > root;

    unsigned cost;
    const unsigned MaxCost;
};

#endif // IMAGECACHE_H
