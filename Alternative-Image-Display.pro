#-------------------------------------------------
#
# Project created by QtCreator 2017-01-14T13:27:08
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Alternative-Image-Display
TEMPLATE = app


SOURCES +=\
        MainWindow.cpp \
    Main.cpp \
    FlowerLayout.cpp \
    FlowLayout.cpp \
    LayoutItem.cpp

HEADERS  += MainWindow.hpp \
    FlowerLayout.hpp \
    FlowLayout.hpp \
    LayoutItem.hpp

FORMS    += MainWindow.ui

INCLUDEPATH += /usr/local/include/opencv

LIBS += -L/usr/local/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs \
