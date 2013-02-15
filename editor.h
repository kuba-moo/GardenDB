#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
    class Editor;
}

class QComboBox;

class BuiltIns;
class Database;
class ImageCache;
class Image;
class Specimen;

/**
 * @brief The Editor class is a editor for species
 */

class Editor : public QWidget
{
    Q_OBJECT

public:
    explicit Editor(Database *db, BuiltIns *builtIns, const QModelIndex &index,
                    QWidget *parent);
    ~Editor();

signals:
    void finished();

private:
    /* Initialization. */
    /* Fill combo with data from given builtins' category. */
    void fillCombo(QComboBox *combo, const QString &category, const unsigned current);

private slots:
    /* Set specimen description. */
    void setDescription();
    /* Fills comboBoxes with built-ins. */
    void populateComboes();
    /* Handle change in one of the comboes. */
    void handleCombo(int);
    /* Reload photos list from images. */
    void reloadPhotos();
    /* Spawn file selection window and append chosen image to images. */
    void addPhoto();
    /* Remove selected photo. */
    void removePhoto();
    /* Set main photo to nth photo in listWidget. */
    void setMainPhoto(int n);
    /* Show image in separate, bigger window. */
    void magnifyImage(QModelIndex);

private:
    Ui::Editor *ui;
    Specimen *specimen;
    ImageCache *ic;
    BuiltIns *builtins;
    QList<Image *> images;
};

#endif // EDITOR_H
