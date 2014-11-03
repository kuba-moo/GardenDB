/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QLinkedList>
#include <QObject>
#include <QVector>
#include <QSize>

class QImage;
class QPixmap;
class QWidget;
class QSqlDatabase;

class LoaderController;
class Image;

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

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject *parent = 0);
    ~ImageCache();

    /* Max size of image that can be generated from cached images. */
    static const QSize UsableCacheSize;

    /* Load images, return true on success. */
    bool load();

    bool isModified();
    int countModified();
    bool save(QSqlDatabase &);

    /* Get specific image. */
    Image *getImage(const int id);
    /* Read form cache pixmap of exactly given size. */
    QPixmap *getPixmap(const int imageId, const QSize &);
    /* Get pixmap greater or equal to given size. */
    QPixmap *getPixmapGe(const int imageId, const QSize &);
    /* Get all pixmaps of given specimen scaled to at least given size. */
    const QList<Image *> &getAllImages(const int spId);

    /* Add new image for specimen. */
    Image *addImage(const int spId, const QString &filename);

signals:
    void changed();
    void oneSaved();

public slots:
    /* Remove image from database and specimen. */
    void removeImage(const int id);

private slots:
    void loadFull();
    void doneCached(int id, QImage *img);
    void doneFullsize(int id, QImage *img);

private:
    /* Image list contains given image. */
    inline bool imageInList(const QList<Image *> &imgs, const int &id) const;

    /* Add image to images vector. */
    int insert(Image *image);
    void insertImages(Image *image);
    void insertSpecies(Image *image);

    int nextInsertId;

    LoaderController *loaderController;
    QVector<QList<Image *> > species;
    QVector<Image *> images;
    QList<Image *> removed;
};

#endif // IMAGECACHE_H
