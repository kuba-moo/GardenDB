#ifndef GALLERY_H
#define GALLERY_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class Gallery;
}

class Database;
class Image;
class Specimen;
class QResizeEvent;

class Gallery : public QWidget
{
    Q_OBJECT
    
public:
    explicit Gallery(Database *db, Specimen *specimen, QWidget *parent = 0);
    ~Gallery();
    
signals:
    void finished();

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void next();
    void previous();
    void setPicture();
    void setPicture(const QModelIndex &index);

private:
    Ui::Gallery *ui;
    int current;
    QList<Image *> images;
};

#endif // GALLERY_H
