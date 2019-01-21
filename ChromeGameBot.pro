#-------------------------------------------------
#
# Project created by QtCreator 2019-01-04T05:07:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChromeGameBot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += D:\OpenCV\opencv\build\include

LIBS += D:\OpenCV\opencv\Release\bin\libopencv_core320.dll
LIBS += D:\OpenCV\opencv\Release\bin\libopencv_highgui320.dll
LIBS += D:\OpenCV\opencv\Release\bin\libopencv_imgcodecs320.dll
LIBS += D:\OpenCV\opencv\Release\bin\libopencv_imgproc320.dll
LIBS += D:\OpenCV\opencv\Release\bin\libopencv_features2d320.dll
LIBS += D:\OpenCV\opencv\Release\bin\libopencv_calib3d320.dll

win32: LIBS += -lUser32


CONFIG += c++11
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    game.cpp \
    area_selector.cpp \
    controller.cpp

HEADERS += \
        mainwindow.h \
    game.h \
    area_selector.h \
    controller.h

FORMS += \
        mainwindow.ui \
    area_selector.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
