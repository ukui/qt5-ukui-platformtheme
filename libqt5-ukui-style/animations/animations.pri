INCLUDEPATH += $$PWD

include(tabwidget/tabwidget.pri)

HEADERS += \
    $$PWD/animator-plugin-iface.h \
    $$PWD/animator-iface.h \
    $$PWD/animation-helper.h

SOURCES += \
    $$PWD/animation-helper.cpp
