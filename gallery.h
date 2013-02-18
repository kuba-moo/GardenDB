#ifndef GALLERY_H
#define GALLERY_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class Gallery;
}

class Database;
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
    /* Image (possibly main) has changed. */
    void imageChanged(const QModelIndex &,const QModelIndex &);
    /* This needs to be posted, before widget is shown scrolls are not visible. */
    void lateInit();

private:
    Ui::Gallery *ui;
    int current;
    int cutWidth;
};

#endif // GALLERY_H
