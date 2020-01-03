#include "proxy-style.h"
#include <QWidget>
#include <QMainWindow>
#include <QWindow>
#include "blur-helper.h"

#include <QDebug>

using namespace UKUI;

ProxyStyle::ProxyStyle(const QString &key) : QProxyStyle (key == nullptr? "breeze": key)
{
    m_blur_helper = new BlurHelper;
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    //FIXME:
    return QProxyStyle::styleHint(hint, option, widget, returnData);
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
