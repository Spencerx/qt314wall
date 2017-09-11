#-------------------------------------------------
#
# Project created by QtCreator 2015-09-13T22:51:00
#
#-------------------------------------------------

QT       += core gui dbus network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXX = /bin/clazy

TARGET = qt314wall
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    source.cpp

HEADERS  += mainwindow.h \
    main.h \
    source.h

FORMS    += mainwindow.ui

RESOURCES += \
    resource.qrc
