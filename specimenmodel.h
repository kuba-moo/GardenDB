#ifndef SPECIMENMODEL_H
#define SPECIMENMODEL_H

#include <QAbstractItemModel>

class Specimen;

class SpecimenModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SpecimenModel(QObject *parent = 0);

    bool load();

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

private:
    QList<Specimen *> specs;
};

#endif // SPECIMENMODEL_H
