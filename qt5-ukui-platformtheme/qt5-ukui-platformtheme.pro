#-------------------------------------------------
#
# Project created by QtCreator 2019-12-31T14:19:01
#
#-------------------------------------------------

QT       += widgets dbus gui-private widgets-private x11extras

greaterThan(QT_MAJOR_VERSION, 5)|greaterThan(QT_MINOR_VERSION, 7): \
    QT += theme_support-private
else: \
    QT += platformsupport-private

TARGET = qt5-ukui-platformtheme
TEMPLATE = lib
CONFIG += plugin
CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt Qt5XdgIconLoader

include(../libqt5-ukui-style/libqt5-ukui-style.pri)

DEFINES += QT5UKUIPLATFORMTHEME_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(widget/widget.pri)

SOURCES += \
        qt5-ukui-platform-theme.cpp \
    main.cpp

HEADERS += \
        qt5-ukui-platform-theme.h \
        qt5-ukui-platformtheme_global.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/platformthemes
    INSTALLS += target
}

DISTFILES += \
    ukui.json
