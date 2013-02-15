#ifndef SPECIMEN_H
#define SPECIMEN_H

#include <QObject>
#include <QSqlRecord>

/**
 * @brief The Specimen class holds information about single specimen
 */

class Specimen : public QObject
{
    Q_OBJECT
public:
    explicit Specimen(const int id, const int no, QObject *parent);
    explicit Specimen(QSqlRecord rec, QObject *parent);
    
    bool isModified() const { return state != Unmodified; }
    bool save();
    bool remove();

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

    /* Setters */
    void setNo(int _no) { no = _no; markModified(); }
    void setMainPhotoId(int _main_photo) { main_photo = _main_photo; markModified(); }
    void setFlavourId(int _fl_id) { fl_id = _fl_id; markModified(); }
    void setFloweringId(int _fw_id) { fw_id = _fw_id; markModified(); }
    void setFrostId(int _fr_id) { fr_id = _fr_id; markModified(); }
    void setTypeId(int _tp_id) { tp_id = _tp_id; markModified(); }
    void setName(const QString & _name) { name = _name; markModified(); }
    void setGrower(const QString & _grower) { grower = _grower; markModified(); }
    void setFlowers(const QString & _flowers) { flowers = _flowers; markModified(); }
    void setSize(const QString & _size) { size = _size; markModified(); }
    void setDesc(const QString & _desc) { desc = _desc; markModified(); }
    
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
