#ifndef FILELOADER_H
#define FILELOADER_H

#include <QImage>
#include <QThread>
#include <QByteArray>

class FileLoader : public QThread
{
    Q_OBJECT
public:
    explicit FileLoader(const QString &filename, QObject *parent = 0);
    
    void run();

    const QString &filename() { return _filename; }
    const QByteArray &raw() { return _raw; }
    const QImage &fullsize() { return _fullsize; }
    const QImage &scaled() { return _scaled; }

private:
    QString _filename;
    QByteArray _raw;
    QImage _fullsize, _scaled;
};

#endif // FILELOADER_H
