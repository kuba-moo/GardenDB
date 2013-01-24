#ifndef ADDNEW_H
#define ADDNEW_H

#include <QWidget>
#include <QSqlRecord>
#include <QModelIndex>

namespace Ui {
    class AddNew;
}

class ImageCache;
class BuiltIns;
class QComboBox;

class AddNew : public QWidget
{
    Q_OBJECT

public:
    explicit AddNew(ImageCache *imageCache, BuiltIns *builtIns,
                    QWidget *parent = 0, const QSqlRecord &record = QSqlRecord());
    ~AddNew();

private:
    /* Initialization. */
    /* Set data values to defaults. */
    void resetData();
    /* Populates data fields with values form record and changes title. */
    void setData(const QSqlRecord& record);
    /* Fill combo with data from given builtins' category. */
    void fillCombo(QComboBox *combo, const QString &category, const unsigned current);

    /* Add contained data to database as new specimen. */
    void acceptAdd();
    /* Update contained specimen. */
    void acceptUpdate();

private slots:
    /* Fills comboBoxes with built-ins. */
    void populateComboes();
    /* Spawn file selection window and append chosen image to images. */
    void addPhoto();
    /* Remove selected photo. */
    void removePhoto();
    /* Set main photo to nth photo in listWidget. */
    void setMainPhoto(int n);
    /* Show image in separate, bigger window. */
    void magnifyImage(QModelIndex);

    /* Add all contained data to database and destroy this tab. */
    void accept();

private:
    bool isNew;
    int speciesId, typeId, flavourId, floweringId, frostId;

    int oldPhotoes;
    Ui::AddNew *ui;
    ImageCache *ic;
    BuiltIns *builtins;
    QVector<int> picIds;
};

#endif // ADDNEW_H
