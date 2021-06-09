QMAKE_CXXFLAGS_RELEASE += -fno-tree-vrp

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = UXcbIntegrationPlugin
!equals(TARGET, $$QT_DEFAULT_QPA_PLUGIN): PLUGIN_EXTENDS = -

DESTDIR = $$_PRO_FILE_PWD_/../bin/plugins/platforms

QT  += opengl x11extras
QT  += core-private
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets widgets-private
    # Qt >= 5.8
    greaterThan(QT_MINOR_VERSION, 7): QT += gui-private
    else: QT += platformsupport-private

    # Qt >= 5.10
    greaterThan(QT_MINOR_VERSION, 9): QT += edid_support-private

    # Qt >= 5.13
    greaterThan(QT_MINOR_VERSION, 12): QT += xkbcommon_support-private
}

TEMPLATE = lib

DEFINES += UXCB_VERSION=\\\"$$VERSION\\\"

linux: include($$PWD/linux.pri)

CONFIG += plugin c++11

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/ukui-platform-integration.cpp \
    $$PWD/ukui-platform-native-interface-hook.cpp \
    $$PWD/ukui-highdpi.cpp

HEADERS += \
    $$PWD/ukui-platform-integration.h \
    $$PWD/ukui-platform-native-interface-hook.h \
    $$PWD/ukui-highdpi.h

INCLUDEPATH += $$PWD/../src

DISTFILES += \
    $$PWD/kylin.json

isEmpty(INSTALL_PATH) {
    target.path = $$[QT_INSTALL_PLUGINS]/platforms
} else {
    target.path = $$INSTALL_PATH
}

message($$target.path)

INSTALLS += target

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT USE_NEW_IMPLEMENTING
} else {
    DEFINES += USE_NEW_IMPLEMENTING
}

contains(DEFINES, USE_NEW_IMPLEMENTING) {
    SOURCES += \
        $$PWD/ukui-platform-backing-store-helper.cpp

    HEADERS += \
        $$PWD/ukui-platform-backing-store-helper.h
}
