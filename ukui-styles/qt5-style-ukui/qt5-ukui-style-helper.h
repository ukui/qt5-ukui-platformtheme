#ifndef QT5UKUISTYLEHELPER_H
#define QT5UKUISTYLEHELPER_H

#include "qt5-ukui-style.h"

void drawComboxPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget);
void drawMenuPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget);
const QRegion getRoundedRectRegion(const QRect &rect, qreal radius_x, qreal radius_y);


#endif // QT5UKUISTYLEHELPER_H
