#-------------------------------------------------
#
# Project created by QtCreator 2020-01-02T08:39:47
#
#-------------------------------------------------

QT       += widgets KWindowSystem x11extras

TARGET = ukui-proxy-style
TEMPLATE = lib
CONFIG += plugin c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

include(../../libqt5-ukui-style/settings/settings.pri)

DEFINES += PROXYSTYLE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gesture-helper.cpp \
        proxy-style.cpp \
    proxy-style-plugin.cpp \
    blur-helper.cpp \
    window-manager.cpp

HEADERS += \
        gesture-helper.h \
        proxy-style.h \
        proxy-style_global.h \ 
    proxy-style-plugin.h \
    blur-helper.h \
    window-manager.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    INSTALLS += target
}

DISTFILES += \
    ukui-style.json
