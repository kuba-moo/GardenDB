#ifndef SPECIMENMODEL_H
#define SPECIMENMODEL_H

#include <QAbstractItemModel>

class ImageCache;
class Specimen;
class QSqlDatabase;

class SpecimenModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SpecimenModel(QObject *parent = 0);

    /* Load information from database, return true on success. */
    bool load(ImageCache *ic);

    bool isModified();
    int countModified();
    bool save(QSqlDatabase &db);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent) const;

    Qt::DropActions supportedDropActions() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool insertColumns(int column, int count, const QModelIndex &parent);
    bool removeColumns(int column, int count, const QModelIndex &parent);

    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);

signals:
    void oneSaved();

private slots:
    void imageChanged();

private:
    bool ignoreRemove;
    int nextInsertId;
    QList<Specimen *> specs, removed;
};

#endif // SPECIMENMODEL_H
