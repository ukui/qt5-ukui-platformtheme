#-------------------------------------------------
#
# Project created by QtCreator 2020-01-03T19:31:51
#
#-------------------------------------------------

QT       += widgets widgets-private

TARGET = qt5-style-ukui
TEMPLATE = lib
CONFIG += plugin c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

include(../../libqt5-ukui-style/libqt5-ukui-style.pri)
include(animations/animations.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    button-animation-helper.cpp \
        qt5-ukui-style-plugin.cpp \
    qt5-ukui-style.cpp \
    tab-widget-animation-helper.cpp \
    scrollbar-animation-helper.cpp \
    qt5-ukui-style-helper.cpp

HEADERS += \
    button-animation-helper.h \
        qt5-ukui-style-plugin.h \
    qt5-ukui-style.h \
    tab-widget-animation-helper.h \
    scrollbar-animation-helper.h \
    qt5-ukui-style-helper.h
DISTFILES += qt5-style-ukui.json 

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    INSTALLS += target
}
