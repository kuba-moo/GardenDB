#include "logger.h"
#include "image.h"
#include "imagecache.h"
#include "imagelistmodel.h"

ImageListModel::ImageListModel(ImageCache *imageCache, const int spId, QObject *parent) :
    QAbstractListModel(parent), ic(imageCache), id(spId),
    imgs(imageCache->getAllImages(spId))
{
    for (int i = 0; i < imgs.size(); i++)
        connect(imgs[i], SIGNAL(changed()), SIGNAL(imageLoaded()));
}

int ImageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return imgs.count();
}

Qt::ItemFlags ImageListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index);
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= imgs.size() || index.column() > 0)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    return (qulonglong)imgs[index.row()];
}

bool ImageListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Log(Debug) << row << count << parent.isValid();

    return true;
}

bool ImageListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    Log(Debug) << "ImageListModel remove" << imgs[row]->id() << "at" << row;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        ic->removeImage(imgs[row]->id());

    endRemoveRows();

    return true;
}

/* This is slightly cheatfull, but we have no way to insert empty image for now
 * so calling insertRows is not an option.
 */
bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid())
        return false;

    if (role != Qt::EditRole && role != Qt::DisplayRole)
        return false;

    beginInsertRows(QModelIndex(), imgs.size(), imgs.size());

    ic->addImage(id, value.toString());
    connect(imgs[imgs.size()-1], SIGNAL(changed()), SIGNAL(imageLoaded()));

    endInsertRows();

    return true;
}

Qt::DropActions ImageListModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::CopyAction;
}

QStringList ImageListModel::mimeTypes() const
{
    QStringList l;
    l << "application/x-qt-image" << "text/plain";
    return l;
}

bool ImageListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                  int row, int column, const QModelIndex &parent)
{
    if (row != -1 || column != -1 || parent.isValid())
        return false;

    if (!data || action != Qt::CopyAction)
        return false;

    Log(Debug) << "Image dropped in with formats" << data->formats();

    if (!data->hasFormat("text/plain"))
        return false;

    QUrl url(data->data("text/plain"));
    if (!url.isLocalFile())
        return false;

    QFileInfo fi(url.toLocalFile());
    if (!fi.exists() || !fi.isReadable())
        return false;

    return setData(QModelIndex(), fi.filePath(), Qt::DisplayRole);
}
