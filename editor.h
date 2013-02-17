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
    explicit Editor(Database *db, const QModelIndex &index, QWidget *parent);
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
    void imageLoaded();
    /* Spawn file selection window and append chosen image to images. */
    void addPhoto();
    /* Remove selected photo. */
    void removePhoto();
    /* Set main photo to nth photo in listWidget. */
    void setMainPhoto(const QModelIndex &index);
    void emitRequestGallery();

private:
    /* Reload photos list from images. */
    void reloadPhotos();

    Ui::Editor *ui;
    Specimen *specimen;
    Builtins *builtins;
};

#endif // EDITOR_H
