#-------------------------------------------------
#
# Project created by QtCreator 2022-12-07T10:28:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network axcontainer xml

TARGET = TranslateToolBox
TEMPLATE = app

DESTDIR = $$absolute_path($$PWD/bin/)

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


INCLUDEPATH += 3rdparty/qtxlsx
INCLUDEPATH += 3rdparty/tinyxml2
INCLUDEPATH += baiduTranslateAPI
INCLUDEPATH += TsFileEdit
INCLUDEPATH += dictionaryEdit
INCLUDEPATH += tsoutput

include($$PWD/3rdparty/qtxlsx/qtxlsx.pri)
include($$PWD/3rdparty/tinyxml2/tinyxml2.pri)

SOURCES += \
        main.cpp \
        toolboxui.cpp \
    baiduTranslateAPI/baidutranslateapi.cpp \
    dictionaryEdit/dicedit.cpp \
    tsoutput/tsoutput.cpp \
    TsFileEdit/tsfileedit.cpp \
    TsFileEdit/tablewidget.cpp \
    TsFileEdit/checkbtn.cpp \
    TsFileEdit/seltlwidget.cpp

HEADERS += \
        toolboxui.h \
    sharefunction.h \
    baiduTranslateAPI/baidutranslateapi.h \
    dictionaryEdit/dicedit.h \
    tsoutput/tsoutput.h \
    TsFileEdit/tsfileedit.h \
    TsFileEdit/tablewidget.h \
    TsFileEdit/checkbtn.h \
    TsFileEdit/seltlwidget.h

FORMS += \
        toolboxui.ui \
    dictionaryEdit/dicedit.ui \
    tsoutput/tsoutput.ui \
    TsFileEdit/tsfileedit.ui

win32::QMAKE_CXXFLAGS += -FIcharsetsetting.inc
win32::QMAKE_CFLAGS += -FIcharsetsetting.inc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES +=
