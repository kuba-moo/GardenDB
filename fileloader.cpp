#include "image.h"
#include "fileloader.h"

#include <QFile>
#include <QBuffer>

FileLoader::FileLoader(const QString &filename, QObject *parent) :
    QThread(parent)
{
    _filename = filename;
}

void FileLoader::run()
{
    QFile file(_filename);
    if (!file.isOpen())
        return;

    _raw = file.readAll();
    QBuffer buf(&_raw);
    buf.open(QIODevice::ReadOnly);

    _fullsize.load(&buf, "JPG");

    if (_fullsize.size().width() < Image::CachedImageSize.width() ||
        _fullsize.size().height() < Image::CachedImageSize.height())
        _scaled = _fullsize;
    else
        _scaled = _fullsize.scaled(Image::CachedImageSize,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
}
