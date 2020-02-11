INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/..

include(tabwidget/tabwidget.pri)
include(scrollbar/scrollbar.pri)

HEADERS += \
    $$PWD/animator-plugin-iface.h \
    $$PWD/animator-iface.h \
    $$PWD/animation-helper.h

SOURCES += \
    $$PWD/animation-helper.cpp
