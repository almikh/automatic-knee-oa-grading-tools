#-------------------------------------------------
#
# Project created by QtCreator 2021-10-24T20:34:08
#
#-------------------------------------------------

QT       += core gui widgets

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

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        classifier.cpp \
        zip/qzip.cpp

HEADERS += \
        mainwindow.h \
        classifier.h \
        zip/qzipreader.h \
        zip/qzipwriter.h

# torch
CONFIG += no_keywords
QMAKE_CXXFLAGS += -DGLIBCXX_USE_CXX11_ABI=0

win32:CONFIG(debug, debug|release): \
    LIBS += -LE:/vhdc/src/3rdparty/libtorch_debug/lib -LD:/Development/automatic-knee-oa-grading-tools/grading_tool/3rdparty/zlib/lib -ltorch -ltorch_cpu -lc10 -lzlibstaticd
else:win32:CONFIG(release, debug|release): \
    LIBS += -LE:/vhdc/src/3rdparty/libtorch_release/lib -LD:/Development/automatic-knee-oa-grading-tools/grading_tool/3rdparty/zlib/lib -ltorch -ltorch_cpu -lc10 -lzlibstatic

INCLUDEPATH += E:/vhdc/src/3rdparty/libtorch_debug/include/
INCLUDEPATH += E:/vhdc/src/3rdparty/libtorch_debug/include/torch/csrc/api/include/
INCLUDEPATH += 3rdparty/zlib/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
