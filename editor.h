/**
 * Copyright (c) 2014 Jakub Kicinski <kubakici@wp.pl>
 * Distributed under the GNU GPL v2. For full terms see the file gpl-2.0.txt.
 **/
#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
    class Editor;
}

class QComboBox;

class Builtins;
class Database;
class Specimen;

/**
 * @brief The Editor class is a editor for species
 */

class Editor : public QWidget
{
    Q_OBJECT

public:
    explicit Editor(Database *db, Specimen *s, QWidget *parent);
    ~Editor();

signals:
    void requestGallery(Specimen *);
    void finished();

private:
    /* Initialization. */
    /* Fill combo with data from given builtins' category. */
    void fillCombo(QComboBox *combo, const QString &category, const int current);

private slots:
    /* Set specimen description. */
    void setDescription();
    /* Fills comboBoxes with built-ins. */
    void populateComboes();
    /* Handle change in one of the comboes. */
    void handleCombo(int);
    /* Image (possibly main) was loaded from disk. */
    void imageChanged(const QModelIndex &,const QModelIndex &);
    /* Spawn file selection window and append chosen image to images. */
    void addPhoto();
    /* Remove selected photo. */
    void removePhoto();
    /* Set main photo to nth photo in listWidget. */
    void setMainPhoto(const QModelIndex &index);
    void emitRequestGallery();
    void mainPhotoClicked();

private:
    /* Reload photos list from images. */
    void reloadPhotos();

    Ui::Editor *ui;
    Specimen *specimen;
    Builtins *builtins;
};

#endif // EDITOR_H
