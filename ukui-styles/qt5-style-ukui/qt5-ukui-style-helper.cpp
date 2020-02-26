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

#include "qt5-ukui-style-helper.h"
#include "ukui-style-settings.h"

#include <QPainter>
#include <QStyleOption>
#include <QWidget>
#include <QPainterPath>

#include <QDebug>

void drawComboxPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,true);
    auto palette = option->palette;
    bool enable = option->state.testFlag(QStyle::State_Enabled);
    bool hover = option->state.testFlag(QStyle::State_MouseOver);
    if (enable) {
        if (hover) {
            painter->setBrush(palette.brush(QPalette::Normal, QPalette::Highlight));
        } else {
            painter->setBrush(palette.brush(QPalette::Normal, QPalette::Base));
        }
    } else {
        painter->setBrush(palette.brush(QPalette::Disabled, QPalette::Base));
    }
    // painter->setFont(QColor(252,255,0));
    painter->drawRoundedRect(option->rect.adjusted(+1,+1,-1,-1),4,4);
    painter->restore();
}

void drawMenuPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget)
{
    //qDebug()<<"draw menu frame"<<option->styleObject<<option->palette;
    QStyleOption opt = *option;
    auto color = opt.palette.color(QPalette::Base);
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        //qDebug()<<opacity;
        color.setAlphaF(opacity);
    }
    opt.palette.setColor(QPalette::Base, color);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen(opt.palette.color(QPalette::Normal, QPalette::Dark), 1);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    //painter->setPen(Qt::transparent);
    painter->setBrush(color);

    QPainterPath path;
    auto region = widget->mask();
    if (region.isEmpty()) {
        path.addRoundedRect(opt.rect.adjusted(1, 1, -1, -1), 6, 6);
    } else {
        path.addRegion(region);
    }

    //painter->drawPolygon(path.toFillPolygon().toPolygon());
    painter->drawPath(path);
    painter->restore();
    return;
}

const QRegion getRoundedRectRegion(const QRect &rect, qreal radius_x, qreal radius_y)
{
    QPainterPath path;
    path.addRoundedRect(rect, radius_x, radius_y);
    return path.toFillPolygon().toPolygon();
}
