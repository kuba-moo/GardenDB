#ifndef SPECIMENMODEL_H
#define SPECIMENMODEL_H

#include <QAbstractItemModel>

class SpecimenModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SpecimenModel(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // SPECIMENMODEL_H
