#-------------------------------------------------
#
# Project created by QtCreator 2016-06-01T13:53:10
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Logging
TEMPLATE = app


SOURCES += main.cpp\
        logging.cpp \
    persistance.cpp \
    automaticclient.cpp \
    manualclient.cpp \
    benchmarkclient.cpp \
    maintenance.cpp

HEADERS  += logging.h \
    persistance.h \
    automaticclient.h \
    manualclient.h \
    benchmarkclient.h \
    maintenance.h

FORMS    += logging.ui \
    automaticclient.ui \
    manualclient.ui \
    maintenance.ui
