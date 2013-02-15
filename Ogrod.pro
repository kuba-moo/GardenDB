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
    imagecache.cpp \
    builtins.cpp \
    image.cpp \
    fileloader.cpp \
    database.cpp \
    logger.cpp \
    loadercontroller.cpp \
    imageloader.cpp \
    specimen.cpp \
    specimenmodel.cpp \
    specimenrenderer.cpp \
    editor.cpp

HEADERS  += omain.h \
    obuiltinseditor.h \
    maintable.h \
    imagecache.h \
    builtins.h \
    image.h \
    fileloader.h \
    database.h \
    logger.h \
    loadercontroller.h \
    imageloader.h \
    specimen.h \
    specimenmodel.h \
    specimenrenderer.h \
    editor.h

FORMS    += omain.ui \
    obuiltinseditor.ui \
    maintable.ui \
    editor.ui

RESOURCES += \
    app.qrc

TRANSLATIONS = garden_pl.ts


