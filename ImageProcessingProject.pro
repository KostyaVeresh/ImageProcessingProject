#-------------------------------------------------
#
# Project created by QtCreator 2017-04-01T20:22:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ImageProcessingProject
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    image.cpp \
    rawImage.cpp \
    grayscaleImage.cpp \
    numArray.cpp \
    ../qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.h \
    image.h \
    rawImage.h \
    grayscaleImage.h \
    numArray.h \
    ../qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp
