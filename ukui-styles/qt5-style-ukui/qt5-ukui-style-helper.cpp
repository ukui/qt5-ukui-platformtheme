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

#include <KWindowEffects>

#include <QApplication>

#include <QDebug>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

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
    int Menu_xRadius = 4;
    int Menu_yRadius = 4;
    int rander = 5;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(option->rect.adjusted(+rander, +rander, -rander, -rander), Menu_xRadius, Menu_yRadius);

    // Draw a black floor
    QPixmap pixmap(option->rect.size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // Blur the black background
    QImage img = pixmap.toImage();
    qt_blurImage(img, Menu_xRadius, false, false);

    // Dig out the center part
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // Shadow rendering
    painter->drawPixmap(option->rect, pixmap, pixmap.rect());

    //That's when I started drawing the frame floor
    QStyleOption opt = *option;
    auto color = opt.palette.color(QPalette::Base);
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        color.setAlphaF(opacity);
    }

    if (qApp->property("blurEnable").isValid()) {
        bool blurEnable = qApp->property("blurEnable").toBool();
        if (!blurEnable) {
            color.setAlphaF(1);
        }
    }

    //if blur effect is not supported, do not use transparent color.
    if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind)) {
        color.setAlphaF(1);
    }

    opt.palette.setColor(QPalette::Base, color);

    QPen pen(opt.palette.color(QPalette::Normal, QPalette::Dark), 1);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(Qt::transparent);
    painter->setBrush(color);

    QPainterPath path;
    auto region = widget->mask();
    if (region.isEmpty()) {
        path.addRoundedRect(opt.rect.adjusted(+rander, +rander, -rander,-rander), Menu_xRadius, Menu_yRadius);
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

qreal calcRadialPos(const QStyleOptionSlider *dial, int postion)
{
    const int currentSliderPosition = dial->upsideDown ? postion : (dial->maximum - postion);
    qreal a = 0;
    if (dial->maximum == dial->minimum)
        a = M_PI / 2;
    else if (dial->dialWrapping)
        a = M_PI * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * M_PI / (dial->maximum - dial->minimum);
    else
        a = (M_PI * 8 - (currentSliderPosition - dial->minimum) * 10 * M_PI / (dial->maximum - dial->minimum)) / 6;
    return a;
}

QPolygonF calcLines(const QStyleOptionSlider *dial, int offset)
{
    QPolygonF poly;
    int width = dial->rect.width();
    int height = dial->rect.height();
    qreal r = qMin(width, height) / 2;

    qreal xc = width / 2;
    qreal yc = height / 2;
    const int ns = dial->tickInterval;
    if (!ns) // Invalid values may be set by Qt Designer.
        return poly;
    int notches = (dial->maximum + ns - 1 - dial->minimum) / ns;
    if (notches <= 0)
        return poly;
    if (dial->maximum < dial->minimum || dial->maximum - dial->minimum > 1000) {
        int maximum = dial->minimum + 1000;
        notches = (maximum + ns - 1 - dial->minimum) / ns;
    }

    poly.resize(2 + 2 * notches);
    int smallLineSize = offset / 2;
    for (int i = 0; i <= notches; ++i) {
        qreal angle = dial->dialWrapping ? M_PI * 3 / 2 - i * 2 * M_PI / notches
                                         : (M_PI * 8 - i * 10 * M_PI / notches) / 6;
        qreal s = qSin(angle);
        qreal c = qCos(angle);
        if (i == 0 || (((ns * i) % (dial->pageStep ? dial->pageStep : 1)) == 0)) {
            poly[2 * i] = QPointF(xc + (r + 1 - offset) * c,
                                  yc - (r + 1 - offset) * s);
            poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
        } else {
            poly[2 * i] = QPointF(xc + (r + 1 - smallLineSize) * c,
                                  yc - (r + 1 - smallLineSize) * s);
            poly[2 * i + 1] = QPointF(xc + r * c, yc -r * s);
        }
    }
    return poly;
}
