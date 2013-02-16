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
    QSqlQuery query(QString("SELECT * FROM Images WHERE id=%1").arg(id));
    if (query.lastError().isValid()) {
        Log(Error) << "getFull" << query.lastError().text();
        emit idle();
        return;
    }

    if (!query.next())
        Log(Error) << "Fullsize image load failed for image" << id
                   << "query was empty";
    QByteArray data = query.record().value("data").toByteArray();

    QImage *img = new QImage();
    if (!img->loadFromData(data))
        Log(Error) << "Fullsize image load failed for image" << id;

    emit doneFullsize(id, img);

    emit idle();
}
