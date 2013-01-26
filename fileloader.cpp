#include "image.h"
#include "fileloader.h"

FileLoader::FileLoader(const QString &filename, QObject *parent) :
    QThread(parent)
{
    fileName = filename;
}

void FileLoader::run()
{
    _fullsize.load(fileName);
    /* Check if image was read successfully. */
    if (_fullsize.isNull())
        return;

    _scaled = _fullsize.scaled(Image::CachedImageSize,
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
}
