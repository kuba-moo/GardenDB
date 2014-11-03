/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#ifndef SPECIMEN_H
#define SPECIMEN_H

#include <QObject>
#include <QSqlRecord>

class QSqlDatabase;

/**
 * @brief The Specimen class holds information about single specimen
 */

class Specimen : public QObject
{
    Q_OBJECT
public:
    explicit Specimen(const int id, const int no, QObject *parent);
    explicit Specimen(QSqlRecord rec, QObject *parent);
    
    /* True if specimen needs to be saved. */
    bool isModified() const { return state != Unmodified; }
    /* True if specimen needs to be kept for saving. */
    bool remove();
    bool save(QSqlDatabase &db);

    /* Getters */
    int getId() const { return id; }
    int getNo() const { return no; }
    int getMainPhotoId() const { return main_photo; }
    int getFlavourId() const { return fl_id; }
    int getFloweringId() const { return fw_id; }
    int getFrostId() const { return fr_id; }
    int getTypeId() const { return tp_id; }
    const QString &getName() const { return name; }
    const QString &getGrower() const { return grower; }
    const QString &getFlowers() const { return flowers; }
    const QString &getSize() const { return size; }
    const QString &getDesc() const { return desc; }

public slots:
    /* Setters */
    void setNo(int _no) { if (no == _no) return; no = _no; markModified(); }
    void setMainPhotoId(int _main_photo) { if (main_photo == _main_photo) return; main_photo = _main_photo; markModified(); }
    void setFlavourId(int _fl_id) { if (fl_id == _fl_id) return; fl_id = _fl_id; markModified(); }
    void setFloweringId(int _fw_id) { if (fw_id == _fw_id) return; fw_id = _fw_id; markModified(); }
    void setFrostId(int _fr_id) { if (fr_id == _fr_id) return; fr_id = _fr_id; markModified(); }
    void setTypeId(int _tp_id) { if (tp_id == _tp_id) return; tp_id = _tp_id; markModified(); }
    void setName(const QString & _name) { if (name == _name) return; name = _name; markModified(); }
    void setGrower(const QString & _grower) { if (grower == _grower) return; grower = _grower; markModified(); }
    void setFlowers(const QString & _flowers) { if (flowers == _flowers) return; flowers = _flowers; markModified(); }
    void setSize(const QString & _size) { if (size == _size) return; size = _size; markModified(); }
    void setDesc(const QString & _desc) { if (desc == _desc) return; desc = _desc; markModified(); }

private:
    void markModified();

    enum State {
        Unmodified,
        New,
        Modified,
        Removed
    } state;

    int id, no, main_photo, fl_id, fw_id, fr_id, tp_id;
    QString name, grower, flowers, size, desc;
};

#endif // SPECIMEN_H
