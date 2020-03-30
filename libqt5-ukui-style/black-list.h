#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <QStringList>

static const QStringList blackAppList() {
    QStringList l;
    l<<"kylin-assistant";
    l<<"kylin-video";
    l<<"ukui-control-center";
    l<<"ubuntu-kylin-software-center.py";
    l<<"kylin-burner";
    return l;
}

#endif // BLACKLIST_H
