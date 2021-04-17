#-------------------------------------------------
#
# Project created by QtCreator 2020-01-02T12:00:35
#
#-------------------------------------------------

QT       += widgets concurrent

TARGET = qt5-ukui-style
TEMPLATE = lib

DEFINES += LIBQT5UKUISTYLE_LIBRARY

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

include(libqt5-ukui-style.pri)

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt

DISTFILES += \
    org.ukui.style.gschema.xml

unix {
    target.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target

    gschema.path = /usr/share/glib-2.0/schemas
    gschema.files += $$PWD/settings/org.ukui.style.gschema.xml
    INSTALLS += gschema

    pkgconfig.path = $$[QT_INSTALL_LIBS]/pkgconfig
    pkgconfig.files += development-files/qt5-ukui.pc
    INSTALLS += pkgconfig
}
