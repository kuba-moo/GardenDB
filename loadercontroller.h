/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#ifndef LOADERCONTROLLER_H
#define LOADERCONTROLLER_H

#include <QThread>
#include <QList>

class ImageLoader;
class QImage;

class LoaderController : public QThread
{
    Q_OBJECT
public:
    explicit LoaderController(QObject *parent = 0);

    /* Fetch images with given id from cache. */
    void getCached(QList<int> l);
    /* Fetch image with given id in full size. */
    void getFullsize(int id);

signals:
    void doneCached(int id, QImage *image);
    void doneFullsize(int id, QImage *image);

protected:
    void run();

private slots:
    void loaderIdle();

private:
    /* Try to send work to loader. */
    void tryToPostWork();

    QList<int> caches, fulls;
    ImageLoader *loader;
    bool idle;
};

#endif // LOADERCONTROLLER_H
