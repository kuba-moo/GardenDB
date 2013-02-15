#include "specimen.h"

#include <QVariant>

Specimen::Specimen(const int id, const int no, QObject *parent) :
    QObject(parent)
{
}

Specimen::Specimen(QSqlRecord rec, QObject *parent) :
    QObject(parent)
{
    id = rec.value("id").toInt();
    no = rec.value("no").toInt();
    name = rec.value("name").toString();
    grower = rec.value("grower").toString();
    flowers = rec.value("flowers").toString();
    size = rec.value("size").toString();
    desc = rec.value("desc").toString();
    main_photo = rec.value("main_photo").toInt();
    fl_id = rec.value("fl_id").toInt();
    fw_id = rec.value("fw_id").toInt();
    fr_id = rec.value("fr_id").toInt();
    tp_id = rec.value("tp_id").toInt();
}

void Specimen::markModified()
{
    if (state == Normal)
        state = Modified;
}
