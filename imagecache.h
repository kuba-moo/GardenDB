#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QLinkedList>
#include <QObject>
#include <QPixmap>
#include <QVector>

#include "oqueries.h"

class QWidget;

/** ImageCache
 *
 * It's not a cache acutally, more like a accessor. You don't insert anything
 * into it and if it doesn't have the object stored it will fetch it from the
 * database.
 *
 * All the elements are scaled with Qt::KeepAspectRatio.
 */

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

    /* Read from cache. */
    QPixmap *getPixmap(const int, const QSize &);

signals:

public slots:

private:
    explicit ImageCache(QObject *parent = 0);
    ImageCache(ImageCache const&);
    void operator=(ImageCache const&);

    /* Can persistent cache be used to generate image. */
    bool canUsePCache(const QSize &size) const
    {
        return size.width() < UsableCacheSize.width() &&
               size.height() < UsableCacheSize.height();
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
