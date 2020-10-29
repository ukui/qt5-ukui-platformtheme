#include "ukuistyleplugin.h"

UKUIStylePlugin::UKUIStylePlugin(QObject *parent) : QStylePlugin(parent)
{
}

QStyle *UKUIStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new QFusionStyle();

    if (key == "ukui3.1-dark") {
        return new UKUIStyle(UKUIStyle::udark);
    } else if (key == "ukui3.1-light") {
        return new UKUIStyle(UKUIStyle::ulight);
    } else if (key == "ukui3.1-default") {
        return new UKUIStyle(UKUIStyle::udefault);
    }
    return new QFusionStyle();
}

const QStringList UKUIStylePlugin::blackList()
{
    return blackAppList();
}
