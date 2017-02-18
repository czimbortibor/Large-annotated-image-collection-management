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
    Main.cpp \
    layouts/FlowLayout.cpp \
    layouts/RingLayout.cpp \
    db/DB.cpp \
    util/CBIR.cpp \
    util/LayoutItem.cpp \
    forms/MainWindow.cpp \
    views/GraphicsView.cpp

HEADERS  +=\
    layouts/AbstractGraphicsLayout.hpp \
    layouts/FlowLayout.hpp \
    layouts/RingLayout.hpp \
    db/DB.hpp \
    util/CBIR.hpp \
    util/LayoutItem.hpp \
    forms/MainWindow.hpp \
    views/GraphicsView.hpp

FORMS    +=\
    forms/MainWindow.ui

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
