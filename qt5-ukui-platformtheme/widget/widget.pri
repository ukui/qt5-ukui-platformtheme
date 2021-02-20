QT += KWindowSystem
CONFIG += c++11 link_pkgconfig no_keywords
PKGCONFIG += peony

INCLUDEPATH += \
    /usr/include/peony-qt/controls/tool-bar/        \
    /usr/include/peony-qt/controls/navigation-bar/  \

HEADERS += \
    $$PWD/file-dialog.h \
    $$PWD/message-box.h \
    $$PWD/xatom-helper.h

SOURCES += \
    $$PWD/file-dialog.cpp \
    $$PWD/message-box.cpp \
    $$PWD/xatom-helper.cpp
