#include "qt5-ukui-style.h"
#include "ukui-style-settings.h"
#include "ukui-tabwidget-default-slide-animator.h"

#include <QStyleOption>
#include <QWidget>
#include <QPainter>

#include "tab-widget-animation-helper.h"

#include <QDebug>

Qt5UKUIStyle::Qt5UKUIStyle(bool dark) : QProxyStyle ("oxygen")
{
    m_tab_animation_helper = new TabWidgetAnimationHelper(this);
}

int Qt5UKUIStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void Qt5UKUIStyle::polish(QWidget *widget)
{
    if (widget->inherits("QMenu")) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (widget->inherits("QTabWidget")) {
        //FIXME: unpolish, extensiable.
        m_tab_animation_helper->registerWidget(widget);
    }

    return QProxyStyle::polish(widget);
}

void Qt5UKUIStyle::unpolish(QWidget *widget)
{
    if (widget->inherits("QMenu")) {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
    }

    if (widget->inherits("QTabWidget")) {
        m_tab_animation_helper->unregisterWidget(widget);
    }

    return QProxyStyle::unpolish(widget);
}

void Qt5UKUIStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    //qDebug()<<"draw PE"<<element;
    switch (element) {
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
    {
        /*!
          \bug
          a "disabled" menu paint and blur in error, i have no idea about that.
          */
        if (widget->isEnabled()) {
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
            painter->setPen(opt.palette.color(QPalette::Window));
            painter->setBrush(color);
            painter->drawRect(opt.rect.adjusted(0, 0, -1, -1));
            painter->restore();
            return;
        }

        return QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
    default:
        break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}
