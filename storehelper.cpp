#include "image.h"
#include "storehelper.h"

#include <QBuffer>
#include <QDebug>

StoreHelper::StoreHelper(QImage img, QObject *parent) :
    QThread(parent), image(img)
{
    t.start();
}

void StoreHelper::run()
{
    QBuffer fs(&_fullsize), cc(&_cache);
    fs.open(QIODevice::WriteOnly);
    image.save(&fs, "PNG");

    cc.open(QIODevice::WriteOnly);
    image.scaled(Image::CachedImageSize,
                 Qt::KeepAspectRatioByExpanding,
                 Qt::SmoothTransformation).save(&cc, "PNG");
    qDebug() << "StoreHelper finished in" << t.elapsed() << "ms";
}
