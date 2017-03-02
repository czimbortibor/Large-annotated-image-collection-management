#-------------------------------------------------
#
# Project created by QtCreator 2017-01-14T13:27:08
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Alternative-Image-Display
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES +=\
    Main.cpp \
    layouts/FlowLayout.cpp \
    util/CBIR.cpp \
    util/LayoutItem.cpp \
    forms/MainWindow.cpp \
    views/GraphicsView.cpp \
    db/MongoAccess.cpp \
    layouts/PetalLayout.cpp

HEADERS  +=\
    layouts/AbstractGraphicsLayout.hpp \
    layouts/FlowLayout.hpp \
    util/CBIR.hpp \
    util/LayoutItem.hpp \
    forms/MainWindow.hpp \
    views/GraphicsView.hpp \
    db/MongoAccess.hpp \
    util/FlowLayoutFactory.hpp \
    util/PetalLayoutFactory.hpp \
    layouts/PetalLayout.hpp \
    util/AbstractLayoutFactory.hpp \
    util/AbstractFilterFactory.hpp

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

RESOURCES += \
    resources.qrc

DISTFILES +=
