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
    FlowLayout.cpp \
    LayoutItem.cpp \
    RingLayout.cpp \
    CBIR.cpp \
    View.cpp \
    DB.cpp

HEADERS  += MainWindow.hpp \
    FlowLayout.hpp \
    LayoutItem.hpp \
    RingLayout.hpp \
    CBIR.hpp \
    View.hpp \
    DB.hpp

FORMS    += MainWindow.ui

INCLUDEPATH += /usr/local/include \
    /usr/local/include/opencv \
    /usr/local/include/mongocxx/v_noabi \
    /usr/local/include/bsoncxx/v_noabi

LIBS += -L/usr/local/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_img_hash \
    -lpHash \
    -lmongocxx \
    -lbsoncxx
