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
    painter->setPen(opt.palette.color(QPalette::Window));
    //painter->setPen(Qt::transparent);
    painter->setBrush(color);

    QPainterPath path;
    auto region = widget->mask();
    if (region.isEmpty()) {
        path.addRoundedRect(opt.rect.adjusted(0, 0, -1, -1), 10, 10);
    } else {
        path.addRegion(region);
    }

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
