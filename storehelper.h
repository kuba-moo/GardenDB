#ifndef STOREHELPER_H
#define STOREHELPER_H

#include <QByteArray>
#include <QImage>
#include <QTime>
#include <QThread>

class StoreHelper : public QThread
{
    Q_OBJECT
public:
    explicit StoreHelper(QImage img, QObject *parent = 0);

    void run();

    const QByteArray &fullsize() {return _fullsize; }
    const QByteArray &cache() { return _cache; }

private:
    QImage image;
    QByteArray _fullsize, _cache;
    QTime t;
};

#endif // STOREHELPER_H
