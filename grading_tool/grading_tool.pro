#-------------------------------------------------
#
# Project created by QtCreator 2021-10-24T20:34:08
#
#-------------------------------------------------

QT += core gui widgets concurrent charts

TARGET = grading_tool
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

include(../dependencies.pri)

CONFIG += c++1z

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        classifier.cpp \
        progress_indicator.cpp \
        tfdetect/tfdetect.cpp \
        utils.cpp \
        view_queue.cpp \
        viewport.cpp \
        zip/qzip.cpp

HEADERS += \
        mainwindow.h \
        classifier.h \
        progress_indicator.h \
        tfdetect/tfdetect.h \
        tfdetect/tfwrapper.h \
        utils.h \
        view_queue.h \
        viewport.h \
        zip/qzipreader.h \
        zip/qzipwriter.h

# torch
# CONFIG += no_keywords
QMAKE_CXXFLAGS += -DGLIBCXX_USE_CXX11_ABI=0

INCLUDEPATH += $${OPENCV_PATH}/include
INCLUDEPATH += $${THIRD_PARTY_PATH}/zlib/include
INCLUDEPATH += $${GDCM_PATH}/include
INCLUDEPATH += $${LIBTENSORFLOW_PATH}/include

LIBS += -L$${THIRD_PARTY_PATH}/zlib/lib/
LIBS += -L$${LIBTENSORFLOW_PATH}/lib -ltensorflow

win32:CONFIG(debug, debug|release) {
    INCLUDEPATH += $${LIBTORCH_PATH}_debug/include $${LIBTORCH_PATH}_debug/include/torch/csrc/api/include
    LIBS += -L$${LIBTORCH_PATH}_debug/lib -ltorch -ltorch_cpu -lc10 -lzlibstaticd
    LIBS += -L$${OPENCV_PATH}/lib -lopencv_world420d
    LIBS += -L$${GDCM_PATH}/lib/Debug -lgdcmcharls -lgdcmDICT -lgdcmDSED -lgdcmexpat -lgdcmgetopt -lgdcmIOD -lgdcmCommon -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmjpeg8 -lgdcmMEXD -lgdcmMSFF -lgdcmopenjp2 -lgdcmzlib -lsocketxx -lws2_32

}
else:win32:CONFIG(release, debug|release) {
    INCLUDEPATH += $${LIBTORCH_PATH}_release/include $${LIBTORCH_PATH}_release/include/torch/csrc/api/include
    LIBS += -L$${LIBTORCH_PATH}_release/lib -ltorch -ltorch_cpu -lc10 -lzlibstatic
    LIBS += -L$${OPENCV_PATH}/lib -lopencv_world420
    LIBS += -L$${GDCM_PATH}/lib/Debug -lgdcmcharls -lgdcmDICT -lgdcmDSED -lgdcmexpat -lgdcmgetopt -lgdcmIOD -lgdcmCommon -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmjpeg8 -lgdcmMEXD -lgdcmMSFF -lgdcmopenjp2 -lgdcmzlib -lsocketxx -lws2_32
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
