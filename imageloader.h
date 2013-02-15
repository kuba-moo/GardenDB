#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QVector>

class QImage;

class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(QObject *parent = 0);
    
    void start();

signals:
    void idle();
    void doneCached(int id, QImage *image);
    void doneFullsize(int id, QImage *image);

public slots:
    /* Fetch images with given id from cache. */
    void getCached(QVector<int> v);
    /* Fetch image with given id in full size. */
    void getFullsize(int id);

private:
};

#endif // IMAGELOADER_H
