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
    specialattribute.cpp \
    imagerenderer.cpp

HEADERS  += omain.h \
    oqueries.h \
    obuiltinseditor.h \
    maintable.h \
    addnew.h \
    specialattribute.h \
    imagerenderer.h

FORMS    += omain.ui \
    obuiltinseditor.ui \
    maintable.ui \
    addnew.ui \
    specialattribute.ui

RESOURCES += \
    app.qrc

TRANSLATIONS = garden_pl.ts


