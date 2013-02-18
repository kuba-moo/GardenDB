#include "image.h"
#include "imagecache.h"
#include "logger.h"
#include "specimen.h"
#include "specimenmodel.h"

#include <QSqlQuery>
#include <QSqlError>

SpecimenModel::SpecimenModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

bool SpecimenModel::load(ImageCache *ic)
{
    QSqlQuery query("SELECT * FROM Species ORDER BY no,id");
    if (query.lastError().isValid()) {
        Log(Error) << "SpecimenModel load" << query.lastError().text();
        return false;
    }

    nextInsertId = 0;
    while (query.next()) {
        Specimen *spec = new Specimen(query.record(), this);
        spec->setNo(specs.size());
        specs.append(spec);
        nextInsertId = qMax(nextInsertId, spec->getId());

        Image *mainImage = ic->getImage(spec->getMainPhotoId());
        if (mainImage)
            connect(mainImage, SIGNAL(changed()), SLOT(imageChanged()));
    }
    nextInsertId++;

    return true;
}

bool SpecimenModel::isModified()
{
    if (removed.size())
        return true;

    for (int i = 0; i < specs.size(); i++)
        if (specs[i]->isModified())
            return true;

    return false;
}

int SpecimenModel::countModified()
{
    int modified = removed.size();

    for (int i = 0; i < specs.size(); i++)
        if (specs[i]->isModified())
            modified++;

    return modified;
}

bool SpecimenModel::save(QSqlDatabase &db)
{
    while (removed.size()) {
        Specimen *spec = removed.takeFirst();
        if (!spec->save(db))
            return false;
        emit oneSaved();
        delete spec;
    }

    for (int i = 0; i < specs.size(); i++)
        if (specs[i]->isModified()) {
            if (!specs[i]->save(db))
                return false;

            emit oneSaved();
        }

    return true;
}

QModelIndex SpecimenModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
}

QModelIndex SpecimenModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool SpecimenModel::hasChildren(const QModelIndex &parent) const
{
    return parent.isValid() ? false : (rowCount() > 0);
}

int SpecimenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return specs.count();
}

int SpecimenModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant SpecimenModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= specs.size() || index.column() > 0)
        return QVariant();

    //if (role == Qt::ToolTipRole)
        //return specs[index.row()]->getDesc();

    if (role != Qt::DisplayRole)
        return QVariant();

    return (qulonglong)specs[index.row()];
}

Qt::ItemFlags SpecimenModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}

bool SpecimenModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << "SpecimenModel setData" << index << value << role;

    if (index.row() >= 0 && index.row() < specs.size()
        && (role == Qt::EditRole || role == Qt::DisplayRole)) {
        delete specs[index.row()];
        specs[index.row()] = (Specimen *)value.toULongLong();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool SpecimenModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        specs.insert(row+r, new Specimen(nextInsertId++, row+r, this));

    Log(Debug) << "SpecimenModel insert" << specs[row]->getId() << "at" << row;

    endInsertRows();

    return true;
}

bool SpecimenModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    Log(Debug) << "SpecimenModel remove" << specs[row]->getId() << "at" << row;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r) {
        if (specs[row]->remove())
            removed.push_back(specs[row]);
        else
            delete specs[row];
        specs.removeAt(row);
    }

    endRemoveRows();

    return true;
}

bool SpecimenModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    qDebug() << "insertRows" << column << count << parent;

    return true;
}

bool SpecimenModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    qDebug() << "removeRows" << column << count << parent;

    return true;
}

Qt::DropActions SpecimenModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

bool SpecimenModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent)
{
    //qDebug() << "dropMimeData" << data << action << row << column << parent;
    if (row < 0 || column != 0 || parent.isValid())
        return false;

    if (!data || action != Qt::MoveAction)
        return false;

    QStringList types = mimeTypes();
    if (types.isEmpty())
        return false;
    QString format = types.at(0);
    if (!data->hasFormat(format))
        return false;

    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    int r, c;
    while (!stream.atEnd()) {
        QMap<int, QVariant> v;
        stream >> r >> c >> v;
    }

    if (r == row || r == row - 1)
        return false;

    Log(Debug) << "SpecimenModel move" << r << " to " << row;

    beginMoveRows(parent, r, r, parent, row);

    Specimen *tmp = specs.takeAt(r);
    specs.insert(r < row ? row - 1 : row, tmp);
    tmp->setNo(row);

    /* Update numbers of species in the middle. */
    int direction = r < row ? 1 : -1;
    while (r != row) {
        specs[r]->setNo(r);
        r += direction;
    }

    endMoveRows();

    return false;
}

void SpecimenModel::imageChanged()
{
    Image *img = qobject_cast<Image *>(QObject::sender());

    int row;
    for (row = 0; row < specs.size(); row++)
        if (specs[row]->getMainPhotoId() == img->id())
            break;

    if (row == specs.size())
        return;

    QModelIndex i = index(row, 0);

    emit dataChanged(i, i);
}
