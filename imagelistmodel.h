#ifndef IMAGELISTMODEL_H
#define IMAGELISTMODEL_H

#include <QAbstractListModel>

class Image;
class ImageCache;

class ImageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ImageListModel(ImageCache *imageCache,
                            const int spId, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool insertColumns(int , int , const QModelIndex &) { return false; }
    bool removeColumns(int , int , const QModelIndex &) { return false; }

    Qt::DropActions supportedDropActions() const;

    QStringList mimeTypes() const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);

private slots:
    void imageChanged();

private:
    ImageCache *ic;
    int id;
    const QList<Image *> &imgs;
};

#endif // IMAGELISTMODEL_H
