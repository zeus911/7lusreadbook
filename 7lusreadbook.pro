#-------------------------------------------------
#
# Project created by QtCreator 2013-12-21T13:10:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 7lusreadbook
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    textout.cpp \
    block.cpp \
    rfaction.cpp \
    dialogabout.cpp \
    dialogoptions.cpp

HEADERS  += mainwindow.h \
    textout.h \
    block.h \
    rfaction.h \
    dialogabout.h \
    dialogoptions.h

FORMS    += mainwindow.ui \
    dialogabout.ui \
    dialogoptions.ui
