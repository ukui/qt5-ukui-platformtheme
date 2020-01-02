#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include "proxy-style_global.h"
#include <QProxyStyle>

namespace UKUI {

/*!
 * \brief The ProxyStyle class
 * \details
 * ProxyStyle is direct UKUI platform theme style for provide style-management
 * in ukui platform.
 *
 * ProxyStyle is not a completed style, and it must be created completedly form another
 * QStyle (exclude itself), such as fusion, oxygen, etc.
 */
class PROXYSTYLESHARED_EXPORT ProxyStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit ProxyStyle(const QString &key);
    virtual ~ProxyStyle() {}

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;
};

}

#endif // PROXYSTYLE_H
