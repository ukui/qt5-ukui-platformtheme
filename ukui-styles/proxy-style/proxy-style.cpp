#include "proxy-style.h"
#include <QWidget>
#include "blur-helper.h"

#include <QStyleOption>
#include "ukui-style-settings.h"

#include <QPainter>

#include <QDebug>

using namespace UKUI;

ProxyStyle::ProxyStyle(const QString &key) : QProxyStyle (key == nullptr? "fusion": key)
{
    m_blur_helper = new BlurHelper;
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    //FIXME:
    switch (hint) {
    case QStyle::SH_Menu_Scrollable: {
        return 1;
    }
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

void ProxyStyle::polish(QWidget *widget)
{
    //FIXME:
    if(!widget)
        return;

    QProxyStyle::polish(widget);

    qDebug()<<widget->metaObject()->className();
    //add exception.

    if (widget->inherits("QMenu")) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (widget->testAttribute(Qt::WA_TranslucentBackground))
        m_blur_helper->registerWidget(widget);
}

void ProxyStyle::unpolish(QWidget *widget)
{
    //FIXME:
    QProxyStyle::unpolish(widget);
}

void ProxyStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    qDebug()<<"draw PE"<<element;
    switch (element) {
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
    {
        /*!
          \bug
          a "disabled" menu paint and blur in error, i have no idea about that.
          */
        if (widget->isEnabled()) {
            qDebug()<<"draw menu frame"<<option->styleObject<<option->palette;
            QStyleOption opt = *option;
            auto color = opt.palette.color(QPalette::Base);
            if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
                auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
                qDebug()<<opacity;
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
