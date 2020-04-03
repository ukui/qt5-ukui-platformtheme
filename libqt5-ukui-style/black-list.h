#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <QStringList>

static const QStringList blackAppList() {
    QStringList l;
    l<<"ukui-control-center";
    l<<"kylin-burner";
    l<<"assistant";
    return l;
}

static const QStringList blackAppListWithBlurHelper(){
    QStringList l;
    l<<"kylin-assistant";
    l<<"kylin-video";
    l<<"ukui-control-center";
    l<<"ubuntu-kylin-software-center.py";
    l<<"kylin-burner";
    return l;
}

#endif // BLACKLIST_H
