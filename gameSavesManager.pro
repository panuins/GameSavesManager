#-------------------------------------------------
#
# Project created by QtCreator 2017-01-30T08:43:28
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gameSavesManager
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    GameDB.cpp \
    Utils.cpp \
    GameInfo.cpp \
    SavesData.cpp \
    SaveDataEditor.cpp

HEADERS  += mainwindow.h \
    GameDB.h \
    Utils.h \
    GameInfo.h \
    SavesData.h \
    SaveDataEditor.h

FORMS    += mainwindow.ui \
    GameInfo.ui \
    SaveDataEditor.ui \
    SavesData.ui
