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

#ifndef QT5UKUISTYLE_H
#define QT5UKUISTYLE_H

#include <private/qfusionstyle_p.h>
#include <QProxyStyle>

class TabWidgetAnimationHelper;
class ScrollBarAnimationHelper;

/*!
 * \brief The Qt5UKUIStyle class
 * \details
 * This class provide the default ukui style in UKUI desktop enviroment.
 */
class Qt5UKUIStyle : public QFusionStyle
{
    Q_OBJECT
public:
    explicit Qt5UKUIStyle(bool dark = false);

    //debuger
    bool eventFilter(QObject *obj, QEvent *e);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette& palette) ;


    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(QStyle::PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const;

    QRect subControlRect(QStyle::ComplexControl control,
                         const QStyleOptionComplex *option,
                         QStyle::SubControl subControl,
                         const QWidget *widget = nullptr) const;

protected:
    QRect scrollBarSubControlRect(QStyle::ComplexControl control,
                                  const QStyleOptionComplex *option,
                                  QStyle::SubControl subControl,
                                  const QWidget *widget = nullptr) const;

    void realSetWindowSurfaceFormatAlpha(const QWidget *widget) const;
    void realSetMenuTypeToMenu(const QWidget *widget) const;
    void drawComBoxIndicator(SubControl which, const QStyleOptionComplex *option,QPainter *painter) const;

private:
    TabWidgetAnimationHelper *m_tab_animation_helper;
    ScrollBarAnimationHelper *m_scrollbar_animation_helper;

    bool m_use_dark_palette = false;
};

#endif // QT5UKUISTYLE_H
