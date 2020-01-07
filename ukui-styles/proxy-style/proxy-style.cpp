#include "proxy-style.h"
#include <QWidget>
#include "blur-helper.h"

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
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}
