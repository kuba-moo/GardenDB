#ifndef FILELOADER_H
#define FILELOADER_H

#include <QImage>
#include <QThread>

class FileLoader : public QThread
{
    Q_OBJECT
public:
    explicit FileLoader(const QString &filename, QObject *parent = 0);
    
    void run();

    const QImage &fullsize() { return _fullsize; }
    const QImage &scaled() { return _scaled; }

private:
    QString fileName;
    QImage _fullsize, _scaled;
};

#endif // FILELOADER_H
