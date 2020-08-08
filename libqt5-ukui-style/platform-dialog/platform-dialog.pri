CONFIG += link_pkgconfig no_keywords
PKGCONFIG += peony

HEADERS += \
    $$PWD/filedialog-sort-filter-proxymodel.h \
    $$PWD/ukui-action-collection.h \
    $$PWD/ukui-diroperator.h \
    $$PWD/ukui-diroperator_p.h \
    $$PWD/ukui-file-widget.h

SOURCES += \
    $$PWD/filedialog-sort-filter-proxymodel.cpp \
    $$PWD/ukui-action-collection.cpp \
    $$PWD/ukui-diroperator.cpp \
    $$PWD/ukui-file-widget.cpp
