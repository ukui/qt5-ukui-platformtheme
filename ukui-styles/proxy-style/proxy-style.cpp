#include "proxy-style.h"

using namespace UKUI;

ProxyStyle::ProxyStyle(const QString &key) : QProxyStyle (key == nullptr? "fusion": key)
{
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    //FIXME:
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}
