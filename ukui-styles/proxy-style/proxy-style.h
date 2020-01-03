#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include "proxy-style_global.h"
#include <QProxyStyle>
#include <QGSettings>

class BlurHelper;

namespace UKUI {

/*!
 * \brief The ProxyStyle class
 * \details
 * ProxyStyle is direct UKUI platform theme style for provide style-management
 * in ukui platform.
 *
 * ProxyStyle is not a completed style, and it must be created completedly form another
 * QStyle (exclude itself), such as fusion, oxygen, etc.
 *
 * UKUI style provide a global blur effect for qt windows, but it does not mean all window
 * will be blurred. In fact, you should make your application window be transparent first.
 * If you do not want your transparent window be blurred, you should add your class to exception,
 * which cached in gsettings org.ukui.style blur-exception-classes.
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

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

private:
    BlurHelper *m_blur_helper;
};

}

#endif // PROXYSTYLE_H
