#include "qt5-ukui-style-plugin.h"
#include "qt5-ukui-style.h"
#include <QProxyStyle>
#include <QDebug>

Qt5UKUIStylePlugin::Qt5UKUIStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
}

QStyle *Qt5UKUIStylePlugin::create(const QString &key)
{
    //FIXME:
    bool dark = false;
    if (key == "ukui-black") {
        qDebug()<<"use ukui-black";
        dark = true;
    } else {
        qDebug()<<"use ukui-white";
    }
    return new Qt5UKUIStyle(dark);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qt5-style-ukui, Qt5UKUIStylePlugin)
#endif // QT_VERSION < 0x050000
