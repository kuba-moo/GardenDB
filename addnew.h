#ifndef ADDNEW_H
#define ADDNEW_H

#include <QWidget>
#include <QSqlRecord>
#include <QModelIndex>

namespace Ui {
    class AddNew;
}

class SpecialAttribute;

class AddNew : public QWidget
{
    Q_OBJECT

public:
    explicit AddNew(QWidget *parent = 0);
    ~AddNew();

public slots:
    /* TODO: use this after built ins change. */
    /* Should be called apon built-ins change. */
    void populateComboes();
    /* Populates data fields with values form record and changes title. */
    void setData(const QSqlRecord& record);

private slots:
    /* Spawn file selection window and append chosen image to images. */
    void addPhoto();
    /* Remove selected photo. */
    void removePhoto();
    /* Set main photo to nth photo in listWidget. */
    void setMainPhoto(int n);
    /* TODO: doc */
    void magnifyImage(QModelIndex);

    /* Add all contained data to database and destroy this tab. */
    void accept();

private:
    bool isNew;
    int speciesId;
    int oldPhotoes;
    Ui::AddNew *ui;
    QVector<int> picIds;
    QList<SpecialAttribute *> specialAttributes;
};

#endif // ADDNEW_H
