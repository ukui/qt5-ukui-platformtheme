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

#ifndef PROXYSTYLE_H
#define PROXYSTYLE_H

#include "proxy-style_global.h"
#include <QProxyStyle>

class BlurHelper;
class WindowManager;

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

    //debuger
    bool eventFilter(QObject *obj, QEvent *e);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

private:
    BlurHelper *m_blur_helper;
    WindowManager *m_window_manager;
};

}

#endif // PROXYSTYLE_H
