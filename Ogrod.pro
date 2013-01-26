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
    oqueries.cpp \
    imagerenderer.cpp \
    imagecache.cpp \
    builtins.cpp \
    image.cpp \
    fileloader.cpp \
    storehelper.cpp

HEADERS  += omain.h \
    oqueries.h \
    obuiltinseditor.h \
    maintable.h \
    addnew.h \
    imagerenderer.h \
    imagecache.h \
    builtins.h \
    image.h \
    fileloader.h \
    storehelper.h

FORMS    += omain.ui \
    obuiltinseditor.ui \
    maintable.ui \
    addnew.ui

RESOURCES += \
    app.qrc

TRANSLATIONS = garden_pl.ts


