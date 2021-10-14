/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef QT5UKUISTYLEHELPER_H
#define QT5UKUISTYLEHELPER_H

#include "qt5-ukui-style.h"
#include <QStyleOption>
#include <qmath.h>

const QRegion getRoundedRectRegion(const QRect &rect, qreal radius_x, qreal radius_y);
qreal calcRadialPos(const QStyleOptionSlider *dial, int postion);
QPolygonF calcLines(const QStyleOptionSlider *dial, int offset);
void tabLayout(const QStyleOptionTab *tab, const QWidget *widget, const QStyle *style, QRect *textRect, QRect *iconRect);
QColor mixColor(const QColor &c1, const QColor &c2, qreal bias = 0.5);
void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton, const QRect &rect, QPainter *painter, const QWidget *widget = 0);
#endif // QT5UKUISTYLEHELPER_H
