#include "specimen.h"
#include "logger.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

Specimen::Specimen(const int _id, const int _no, QObject *parent) :
    QObject(parent)
{
    state = New;

    id = _id;
    no = _no;
    name = grower = flowers = size = desc = QString();
    main_photo = fl_id = fw_id = fr_id = tp_id = 0;
}

Specimen::Specimen(QSqlRecord rec, QObject *parent) :
    QObject(parent)
{
    state = Unmodified;

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

bool Specimen::remove()
{
    if (state == New)
        return false;

    state = Removed;
    return true;
}

bool Specimen::save(QSqlDatabase &db)
{
    QString str;

    if (state == New)
        str = "INSERT INTO Species(id,no,name,grower,flowers,size,desc,"
                "main_photo,fl_id,fw_id,fr_id,tp_id) VALUES(" +
                QString::number(id) + "," +
                QString::number(no) + "," +
                "'" + name + "'," +
                "'" + grower + "'," +
                "'" + flowers + "'," +
                "'" + size + "'," +
                "'" + desc + "'," +
                QString::number(main_photo) + "," +
                QString::number(fl_id) + "," +
                QString::number(fw_id) + "," +
                QString::number(fr_id) + "," +
                QString::number(tp_id) + ')';
    else if (state == Modified)
        str = "UPDATE Species SET "
                "no=" + QString::number(no) + ","
                "name='" + name + "',"
                "grower='" + grower + "',"
                "flowers='" + flowers + "',"
                "size='" + size + "',"
                "desc='" + desc + "',"
                "main_photo=" + QString::number(main_photo) + ","
                "fl_id=" + QString::number(fl_id) + ","
                "fw_id=" + QString::number(fw_id) + ","
                "fr_id=" + QString::number(fr_id) + ","
                "tp_id=" + QString::number(tp_id) +
                " WHERE id=" + QString::number(id);
    else if (state == Removed)
        str = "DELETE FROM Species WHERE id=" + QString::number(id);
    else
        Log(Assert) << "Someone saved unmodified Specimen";

    QSqlQuery query(str, db);
    if (query.lastError().isValid()) {
        Log(Error) << query.lastError().text() << " | " << query.lastQuery();
        return false;
    }
    if (query.numRowsAffected() != 1)
        Log(Warning) << "Specimen save Rows affected"
                     << query.numRowsAffected() << "state" << state;

    if (state != Removed)
        state = Unmodified;
    return true;
}

void Specimen::markModified()
{
    if (state == Removed)
        qDebug() << "Huh?? Removed species alive?";
    if (state == Unmodified)
        state = Modified;
}
