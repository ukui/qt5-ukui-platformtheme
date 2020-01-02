#include "proxy-style-plugin.h"
#include "proxy-style.h"

#include <QDebug>

using namespace UKUI;

QStyle *ProxyStylePlugin::create(const QString &key)
{
    if (key == "ukui") {
        //FIXME:
        //get current style, fusion for invalid.
        qDebug()<<"ukui create proxy style";
        return new ProxyStyle(nullptr);
    }
    qDebug()<<"ukui create proxy style: null";
    return nullptr;
}
