INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/..

include(tabwidget/tabwidget.pri)
include(scrollbar/scrollbar.pri)
include(button/button-animator.pri)

HEADERS += \
    $$PWD/animator-plugin-iface.h \
    $$PWD/animator-iface.h \
    $$PWD/animation-helper.h

SOURCES += \
    $$PWD/animation-helper.cpp
