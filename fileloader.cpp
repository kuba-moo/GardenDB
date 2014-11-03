/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#include "image.h"
#include "fileloader.h"

#include <QFile>
#include <QDebug>

FileLoader::FileLoader(const QString &filename, QObject *parent) :
    QThread(parent)
{
    _filename = filename;
}

void FileLoader::run()
{
    QFile file(_filename);
    if (!file.open(QFile::ReadOnly))
        return;

    _raw = file.readAll();
    _fullsize.loadFromData(_raw);

    if (_fullsize.size().width() < Image::CachedImageSize.width() ||
        _fullsize.size().height() < Image::CachedImageSize.height())
        _scaled = _fullsize;
    else
        _scaled = _fullsize.scaled(Image::CachedImageSize,
                                   Qt::KeepAspectRatioByExpanding,
                                   Qt::SmoothTransformation);
}
