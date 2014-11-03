#-------------------------------------------------
#
# Project created by QtCreator 2011-03-21T16:54:31
#
#-------------------------------------------------

QT       += core gui sql

TARGET = garden
TEMPLATE = app

CONFIG += rtti

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
    editor.cpp \
    builtinvalue.cpp \
    savingthread.cpp \
    savingwidget.cpp \
    gallery.cpp \
    imagelistrenderer.cpp \
    imagelistmodel.cpp

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
    editor.h \
    builtinvalue.h \
    savingthread.h \
    savingwidget.h \
    gallery.h \
    imagelistrenderer.h \
    imagelistmodel.h

FORMS    += omain.ui \
    obuiltinseditor.ui \
    maintable.ui \
    editor.ui \
    savingwidget.ui \
    gallery.ui

RESOURCES += \
    app.qrc

TRANSLATIONS = garden_pl.ts

OTHER_FILES += \
    win_app.rc \
    garden-install.nsi \
    TODO

win32:RC_FILE = win_app.rc
