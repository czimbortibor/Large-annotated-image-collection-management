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
    layouts/PetalLayout.cpp \
    layouts/SpiralLayout.cpp \
    ui/MainWindow.cpp \
    view/GraphicsView.cpp \
    filters/DateFilter.cpp \
    utils/CBIR.cpp \
    utils/LayoutItem.cpp \
    utils/ConfigurationsHandler.cpp \
    utils/ImageConverter.cpp \
    utils/image_load/ImageLoader.cpp \
    utils/image_load/LoadingHandler.cpp \
    utils/ImageCollection.cpp \
    db/DbContext.cpp

HEADERS  +=\
    layouts/AbstractGraphicsLayout.hpp \
    layouts/FlowLayout.hpp \
    layouts/PetalLayout.hpp \
    layouts/SpiralLayout.hpp \
    ui/MainWindow.hpp \
    view/GraphicsView.hpp \
    filters/DateFilter.hpp \
    filters/AbstractFilter.hpp \
    utils/AbstractFilter.hpp \
    utils/AbstractLayoutFactory.hpp \
    utils/FlowLayoutFactory.hpp \
    utils/PetalLayoutFactory.hpp \
    utils/CBIR.hpp \
    utils/LayoutItem.hpp \
    utils/ImageConverter.hpp \
    utils/ConfigurationsHandler.hpp \
    utils/image_load/ImageLoader.hpp \
    utils/image_load/Mapper.hpp \
    utils/image_load/Reducer.hpp \
    utils/image_load/LoadingHandler.hpp \
    utils/ImageCollection.hpp \
    utils/SpiralLayoutFactory.hpp \
    db/DbContext.hpp

FORMS    +=\
    ui/MainWindow.ui

RESOURCES += \
    resources.qrc

DISTFILES +=

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/release/ \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_img_hash \
    -lpHash \
    -lmongocxx \
    -lbsoncxx \
    -lconfig++
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib/debug/ \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_img_hash \
    -lpHash \
    -lmongocxx \
    -lbsoncxx \
    -lconfig++
else:unix: LIBS += -L$$PWD/../../../../usr/local/lib/ \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_img_hash \
    -lpHash \
    -lmongocxx \
    -lbsoncxx \
    -lconfig++

INCLUDEPATH += $$PWD/../../../../usr/local/include \
               $$PWD/../../../../usr/local/include/opencv \
               $$PWD/../../../../usr/local/include/mongocxx/v_noabi \
               $$PWD/../../../../usr/local/include/bsoncxx/v_noabi

DEPENDPATH += $$PWD/../../../../usr/local/include \
              $$PWD/../../../../usr/local/include/opencv \
              $$PWD/../../../../usr/local/include/mongocxx/v_noabi \
              $$PWD/../../../../usr/local/include/bsoncxx/v_noabi
