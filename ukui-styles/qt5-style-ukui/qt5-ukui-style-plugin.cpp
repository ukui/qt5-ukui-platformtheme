#include "qt5-ukui-style-plugin.h"
#include <QProxyStyle>
#include <QDebug>

Qt5UKUIStylePlugin::Qt5UKUIStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
}

QStyle *Qt5UKUIStylePlugin::create(const QString &key)
{
    //FIXME:
    if (key == "ukui-black") {
        qDebug()<<"use ukui-black";
    } else {
        qDebug()<<"use ukui-white";
    }
    return new QProxyStyle("fusion");
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qt5-style-ukui, Qt5UKUIStylePlugin)
#endif // QT_VERSION < 0x050000
