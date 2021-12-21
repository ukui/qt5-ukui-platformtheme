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

#include <QProxyStyle>

class TabWidgetAnimationHelper;
class ScrollBarAnimationHelper;
class ButtonAnimationHelper;
class BoxAnimationHelper;
class ProgressBarAnimationHelper;
class ShadowHelper;

class QStyleOptionViewItem;
class QDBusInterface;

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
#include<private/qfusionstyle_p.h>
#define Style QFusionStyle
#else
#define Style QProxyStyle
#endif

/*!
 * \brief The Qt5UKUIStyle class
 * \details
 * This class provide the default ukui style in UKUI desktop enviroment.
 */
class Qt5UKUIStyle : public Style
{
    Q_OBJECT
public:
    explicit Qt5UKUIStyle(bool dark = false, bool useDefault = true);

    const QStringList specialList() const;

    bool shouldBeTransparent(const QWidget *w) const;

    //debuger
    bool eventFilter(QObject *obj, QEvent *e);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette& palette) ;

    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
                           const QWidget *widget = nullptr) const override;

    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

    QPalette standardPalette() const;

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

    void drawItemPixmap(QPainter *painter,
                        const QRect &rect,
                        int alignment,
                        const QPixmap &pixmap) const;

    QRect subElementRect(SubElement element,
                                 const QStyleOption *option,
                                 const QWidget *widget = nullptr) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;

protected:
    const QStringList useDefaultPalette() const;
    void viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const;

    void realSetWindowSurfaceFormatAlpha(const QWidget *widget) const;
    void realSetMenuTypeToMenu(const QWidget *widget) const;
    QRect centerRect(const QRect &rect, int width, int height) const;

private:
    TabWidgetAnimationHelper *m_tab_animation_helper;
    ScrollBarAnimationHelper *m_scrollbar_animation_helper;
    ButtonAnimationHelper*  m_button_animation_helper;
    BoxAnimationHelper* m_combobox_animation_helper;
    ShadowHelper *m_shadow_helper;
    ProgressBarAnimationHelper *m_animation_helper;

    /*!
     * \brief m_use_dark_palette
     * \deprecated
     * use qApp->property("preferDark") instead. link to: #63026.
     */
    bool m_use_dark_palette = false;
    bool m_is_default_style = true;

    bool m_is_tablet_mode = false;
    QDBusInterface *m_statusManagerDBus = nullptr;

    QColor button_Click(const QStyleOption *option) const;
    QColor button_Hover(const QStyleOption *option) const;
    QColor button_DisableChecked() const;
    QColor highLight_Click(const QStyleOption *option) const;
    QColor highLight_Hover(const QStyleOption *option) const;

private Q_SLOTS:
    void updateTabletModeValue(bool isTabletMode);
};

#endif // QT5UKUISTYLE_H
