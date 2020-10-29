#include "ukuistyle.h"

UKUIStyle::UKUIStyle(QObject *parent)
    : QStylePlugin(parent)
{
}

QStyle *UKUIStyle::create(const QString &key)
{
    static_assert(false, "You need to implement this function");
}
