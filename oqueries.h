#ifndef OQUERIES_H
#define OQUERIES_H

#include <QApplication>
#include <QString>
#include <QSize>

/**
 * Static declarations of query strings.
 */

/* Commands used to create database. */
extern const unsigned int numCreates;
extern const QString creates[];

extern const QSize CachedImageSize;
extern const QSize UsableCacheSize;
extern const QSize ImageSize;

/* Commands initializing configuration stored in database to defaults. */
extern const unsigned int numInserts;
extern const QString inserts[];

/* Reading queries. */
extern const QString getTypes;
extern const QString getFlavours;
extern const QString getFlowering;
extern const QString getFrost;

#endif // OQUERIES_H
