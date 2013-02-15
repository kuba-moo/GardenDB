#-------------------------------------------------
#
# Project created by QtCreator 2011-03-21T16:54:31
#
#-------------------------------------------------

QT       += core gui sql

TARGET = Ogrod
TEMPLATE = app


SOURCES += main.cpp\
        omain.cpp \
    obuiltinseditor.cpp \
    maintable.cpp \
    addnew.cpp \
    imagecache.cpp \
    builtins.cpp \
    image.cpp \
    fileloader.cpp \
    storehelper.cpp \
    database.cpp \
    logger.cpp \
    loadercontroller.cpp \
    imageloader.cpp \
    specimen.cpp \
    specimenmodel.cpp \
    specimenrenderer.cpp

HEADERS  += omain.h \
    obuiltinseditor.h \
    maintable.h \
    addnew.h \
    imagecache.h \
    builtins.h \
    image.h \
    fileloader.h \
    storehelper.h \
    database.h \
    logger.h \
    loadercontroller.h \
    imageloader.h \
    specimen.h \
    specimenmodel.h \
    specimenrenderer.h

FORMS    += omain.ui \
    obuiltinseditor.ui \
    maintable.ui \
    addnew.ui

RESOURCES += \
    app.qrc

TRANSLATIONS = garden_pl.ts


