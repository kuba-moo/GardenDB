#include "imageloader.h"
#include "logger.h"

#include <QImage>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

ImageLoader::ImageLoader(QObject *parent) :
    QObject(parent)
{
}

void ImageLoader::start()
{
    emit idle();
}

void ImageLoader::getCached(QVector<int> v)
{
    QString str("SELECT * FROM ImagesCache WHERE ");
    for (int i = 0; i < v.size(); i++) {
        if (i)
            str.append(" OR ");
        str.append(QString("id=%1").arg(v[i]));
    }
    QSqlQuery query(str);
    if (query.lastError().isValid()) {
        Log(Error) << "getCached" << query.lastError().text();
        emit idle();
        return;
    }

    while (query.next()) {
        int id = query.record().value("id").toInt();
        QByteArray data = query.record().value("data").toByteArray();

        QImage *img = new QImage();
        img->loadFromData(data, "JPG");

        emit doneCached(id, img);
    }

    emit idle();
}

void ImageLoader::getFullsize(int id)
{
    QSqlQuery query(QString("SELECT * FROM ImagesCache WHERE id=%1").arg(id));
    if (query.lastError().isValid()) {
        Log(Error) << "getFull" << query.lastError().text();
        emit idle();
        return;
    }

    QByteArray data = query.record().value("data").toByteArray();

    QImage *img = new QImage();
    img->loadFromData(data, "JPG");

    emit doneFullsize(id, img);

    emit idle();
}
