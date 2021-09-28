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

#include "qt5-ukui-style.h"

#include "qt5-ukui-style-helper.h"

#include "ukui-style-settings.h"
#include "ukui-tabwidget-default-slide-animator.h"

#include <QStyleOption>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include <QPaintDevice>

#include "tab-widget-animation-helper.h"
#include "scrollbar-animation-helper.h"
#include "button-animation-helper.h"
#include "button-animator.h"
#include "box-animation-helper.h"
#include "animator-iface.h"
#include "animation-helper.h"
#include "progressbar-animation-helper.h"
#include "progressbar-animation.h"
#include "shadow-helper.h"

#include "highlight-effect.h"

#include <QIcon>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTreeView>
#include <QListView>
#include <QMenu>
#include <QToolButton>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QEvent>
#include <QDebug>
#include <QPixmapCache>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QApplication>

#include <private/qlineedit_p.h>

#define COMMERCIAL_VERSION true
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : 0;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
Qt5UKUIStyle::Qt5UKUIStyle(bool dark, bool useDefault) : QFusionStyle()
#else
Qt5UKUIStyle::Qt5UKUIStyle(bool dark, bool useDefault) : QProxyStyle("fusion")
#endif
{
    m_is_default_style = useDefault;
    m_use_dark_palette = dark;
    m_tab_animation_helper = new TabWidgetAnimationHelper(this);
    m_scrollbar_animation_helper = new ScrollBarAnimationHelper(this);
    m_button_animation_helper = new ButtonAnimationHelper(this);
    m_combobox_animation_helper = new BoxAnimationHelper(this);
    m_animation_helper = new ProgressBarAnimationHelper(this);
    m_shadow_helper = new ShadowHelper(this);

    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString themeColor = settings->get("themeColor").toString();
        QPalette palette = qApp->palette();
        setThemeColor(themeColor, palette);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "themeColor") {
                QString themeColor = settings->get("themeColor").toString();
                QPalette palette = qApp->palette();
                setThemeColor(themeColor, palette);
                qApp->setPalette(palette);
                emit qApp->paletteChanged(palette);
            }
        });
    } else {
        mHighLightClick = QColor(36, 109, 212);
        mHighLightHover = QColor(55, 144, 250);
    }
}

const QStringList Qt5UKUIStyle::specialList() const
{
    //use dark palette in default style.
    QStringList l;
    l<<"ukui-menu";
    l<<"ukui-panel";
    l<<"ukui-sidebar";
    l<<"ukui-volume-control-applet-qt";
    l<<"ukui-power-manager-tray";
    l<<"kylin-nm";
    l<<"ukui-flash-disk";
//    l<<"ukui-bluetooth";
    l<<"mktip";
    return l;
}

const QStringList Qt5UKUIStyle::useDefaultPalette() const
{
    QStringList l;
    l<<"kybackup";
    l<<"biometric-manager";
    return l;
}

bool Qt5UKUIStyle::shouldBeTransparent(const QWidget *w) const
{
    bool should = false;

    if (w->inherits("QComboBoxPrivateContainer"))
        return true;

    if (w->inherits("QTipLabel"))
        return true;

    return should;
}

bool Qt5UKUIStyle::eventFilter(QObject *obj, QEvent *e)
{
    /*!
      \bug
      There is a bug when use fusion as base style when in qt5 assistant's
      HelperView. ScrollBar will not draw with our overrided function correctly,
      and then it will trigger QEvent::StyleAnimationUpdate. by some how it will let
      HelperView be hidden.
      I eat this event to aviod this bug, but the scrollbar in HelperView still
      display with old fusion style, and the animation will be ineffective.

      I don't know why HelperView didn't use our function drawing scrollbar but fusion, that
      makes me stranged.
      */
    if (e->type() == QEvent::StyleAnimationUpdate) {
        return true;
    }

    if (qobject_cast<QPushButton *>(obj) || qobject_cast<QToolButton *>(obj)) {
        if (e->type() == QEvent::Hide) {
            if (QWidget *w = qobject_cast<QWidget *>(obj)) {
                auto animator = m_button_animation_helper->animator(w);
                if (animator) {
                    animator->stopAnimator("SunKen");
                    animator->stopAnimator("MouseOver");
                    animator->setAnimatorCurrentTime("SunKen", 0);
                    animator->setAnimatorCurrentTime("MouseOver", 0);
                }
            }
        }
        return false;
    }
    return false;
}

/*
 * Note there are some widgets can not be set as transparent one in polish.
 * Because it has been created as a rgb window.
 *
 * To reslove this problem, we have to let attribute be setted more ahead.
 * Some styleHint() methods are called in the early creation of a widget.
 * So we can real set them as alpha widgets.
 */
int Qt5UKUIStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    realSetWindowSurfaceFormatAlpha(widget);
    realSetMenuTypeToMenu(widget);

    switch (hint) {
    case SH_ScrollBar_Transient:
        return false;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return int(true);
    case SH_DialogButtons_DefaultButton:
        return int(true);
    case SH_UnderlineShortcut:
        return true;
    case SH_ComboBox_Popup:
        return false;
    case SH_ComboBox_AllowWheelScrolling:
        return int(false);

//    case SH_Button_FocusPolicy:
//        return Qt::TabFocus;

    case SH_Header_ArrowAlignment:
        return Qt::AlignRight | Qt::AlignVCenter;

    case SH_Table_GridLineColor:
        return option ? option->palette.color(QPalette::Active, QPalette::Midlight).rgb() : 0;

    default:
        break;
    }
    return Style::styleHint(hint, option, widget, returnData);
}

void Qt5UKUIStyle::polish(QPalette &palette){
    palette = standardPalette();

    return Style::polish(palette);
}

QPalette Qt5UKUIStyle::standardPalette() const
{
    auto palette = Style::standardPalette();
    QColor windowText_at(38, 38, 38),
           windowText_iat(38, 38, 38),
           windowText_dis(166, 166, 166),
           button_at(230, 230, 230),
           button_iat(230, 230, 230),
           button_dis(233, 233, 233),
           light_at(255, 255, 255),
           light_iat(255, 255, 255),
           light_dis(242, 242, 242),
           midlight_at(217, 217, 217),
           midlight_iat(217, 217, 217),
           midlight_dis(230, 230, 230),
           dark_at(77, 77, 77),
           dark_iat(77, 77, 77),
           dark_dis(64, 64, 64),
           mid_at(115, 115, 115),
           mid_iat(115, 115, 115),
           mid_dis(102, 102, 102),
           text_at(38, 38, 38),
           text_iat(38, 38, 38),
           text_dis(140, 140, 140),
           brightText_at(89, 89, 89),
           brightText_iat(89, 89, 89),
           brightText_dis(77, 77, 77),
           buttonText_at(38, 38, 38),
           buttonText_iat(38, 38, 38),
           buttonText_dis(179, 179, 179),
           base_at(255, 255, 255),
           base_iat(245, 245, 245),
           base_dis(237, 237, 237),
           window_at(245, 245, 245),
           window_iat(237, 237, 237),
           window_dis(230, 230, 230),
           shadow_at(214, 214, 214),
           shadow_iat(214, 214, 214),
           shadow_dis(201, 201, 201),
           highLight_at(55, 144, 250),
           highLight_iat(55, 144, 250),
           hightight_dis(233, 233, 233),
           highLightText_at(255, 255, 255),
           highLightText_iat(255, 255, 255),
           highLightText_dis(179, 179, 179),
           link_at(55, 144, 250),
           link_iat(55, 144, 250),
           link_dis(55, 144, 250),
           linkVisited_at(114, 46, 209),
           linkVisited_iat(114, 46, 209),
           linkVisited_dis(114, 46, 209),
           alternateBase_at(244, 245, 245),
           noRale_at(240, 240, 240),
           noRole_iat(240, 240, 240),
           noRole_dis(217, 217, 217),
           toolTipBase(255, 255, 255),
           toolTipText(38, 38, 38),
           placeholderText(38, 38, 38);

    if (!useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        windowText_at.setRgb(217, 217, 217);
        windowText_iat.setRgb(217, 217, 217);
        windowText_dis.setRgb(77, 77, 77);
        button_at.setRgb(55, 55, 59);
        button_iat.setRgb(55, 55, 59);
        button_dis.setRgb(46, 46, 48);
        midlight_at.setRgb(77, 77, 77);
        midlight_iat.setRgb(77, 77, 77);
        midlight_dis.setRgb(64, 64, 64);
        dark_at.setRgb(38, 38, 38);
        dark_iat.setRgb(38, 38, 38);
        dark_dis.setRgb(26, 26, 26);
        text_at.setRgb(217, 217, 217);
        text_iat.setRgb(217, 217, 217);
        text_dis.setRgb(77, 77, 77);
        brightText_at.setRgb(255, 255, 255);
        brightText_iat.setRgb(255, 255, 255);
        brightText_dis.setRgb(77, 77, 77);
        buttonText_at.setRgb(217, 217, 217);
        buttonText_iat.setRgb(217, 217, 217);
        buttonText_dis.setRgb(76, 76, 79);
        base_at.setRgb(18, 18, 18);
        base_iat.setRgb(28, 28, 28);
        base_dis.setRgb(36, 36, 36);
        window_at.setRgb(35, 36, 38);
        window_iat.setRgb(26, 26, 26);
        window_dis.setRgb(18, 18, 18);
        hightight_dis.setRgb(46, 46, 48);
        highLightText_dis.setRgb(77, 77, 77);
        alternateBase_at.setRgb(38, 38, 38);
        noRale_at.setRgb(51, 51, 51);
        noRole_iat.setRgb(51, 51, 51);
        noRole_dis.setRgb(60, 60, 64);
        toolTipBase.setRgb(38, 38, 38);
        toolTipText.setRgb(217, 217, 217);
        placeholderText.setRgb(166, 166, 166);
    }

    palette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    palette.setColor(QPalette::Active, QPalette::Button, button_at);
    palette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    palette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    palette.setColor(QPalette::Active, QPalette::Light, light_at);
    palette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    palette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    palette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    palette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    palette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    palette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    palette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    palette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    palette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    palette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    palette.setColor(QPalette::Active, QPalette::Text, text_at);
    palette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    palette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    palette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    palette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    palette.setColor(QPalette::Active, QPalette::Base, base_at);
    palette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    palette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    palette.setColor(QPalette::Active, QPalette::Window, window_at);
    palette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    palette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    palette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    palette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    palette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    palette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, hightight_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::Link, link_at);
    palette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    palette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    palette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    palette.setColor(QPalette::AlternateBase, alternateBase_at);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::ToolTipBase, toolTipBase);

    palette.setColor(QPalette::ToolTipText, toolTipText);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::PlaceholderText, placeholderText);
#endif

    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString themeColor = settings->get("themeColor").toString();
        setThemeColor(themeColor, palette);
    } else {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(55, 144, 250));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(55, 144, 250));
        mHighLightClick = QColor(36, 109, 212);
        mHighLightHover = QColor(55, 144, 250);
    }

    return palette;
}



QColor Qt5UKUIStyle::button_Click() const
{
    if (!useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(75, 75, 79);
    } else {
        return QColor(217, 217, 217);
    }
}



QColor Qt5UKUIStyle::button_Hover() const
{
    if (!useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(55, 55, 59);
    } else {
        return QColor(230, 230, 230);
    }
}



QColor Qt5UKUIStyle::button_DisableChecked() const
{
    if (!useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool() || (m_is_default_style && specialList().contains(qAppName())))) {
        return QColor(61, 61, 64);
    } else {
        return QColor(224, 224, 224);
    }
}



QColor Qt5UKUIStyle::highLight_Click() const
{
    return mHighLightClick;
}



QColor Qt5UKUIStyle::highLight_Hover() const
{
    return mHighLightHover;
}



void Qt5UKUIStyle::setThemeColor(QString themeColor, QPalette &palette) const
{
    if (themeColor == "daybreakBlue") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(55, 144, 250));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(55, 144, 250));
        mHighLightClick = QColor(36, 109, 212);
        mHighLightHover = QColor(55, 144, 250);
    } else if (themeColor == "jamPurple") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 46, 209));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(114, 46, 209));
        mHighLightClick = QColor(114, 46, 209);
        mHighLightHover = QColor(83, 29, 171);
    } else if (themeColor == "magenta") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(235, 48, 150));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(235, 48, 150));
        mHighLightClick = QColor(196, 29, 127);
        mHighLightHover = QColor(235, 48, 150);
    } else if (themeColor == "sunRed") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(243, 34, 45));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(243, 34, 45));
        mHighLightClick = QColor(204, 18, 34);
        mHighLightHover = QColor(243, 34, 45);
    } else if (themeColor == "sunsetOrange") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(246, 140, 39));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(246, 140, 39));
        mHighLightClick = QColor(207, 105, 23);
        mHighLightHover = QColor(246, 140, 39);
    } else if (themeColor == "dustGold") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(249, 197, 61));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(249, 197, 61));
        mHighLightClick = QColor(212, 157, 40);
        mHighLightHover = QColor(249, 197, 61);
    } else if (themeColor == "polarGreen") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(82, 196, 41));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(82, 196, 41));
        mHighLightClick = QColor(56, 158, 25);
        mHighLightHover = QColor(82, 196, 41);
    } else {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(55, 144, 250));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(55, 144, 250));
        mHighLightClick = QColor(36, 109, 212);
        mHighLightHover = QColor(55, 144, 250);
    }
}



void Qt5UKUIStyle::polish(QWidget *widget)
{
    Style::polish(widget);

    m_shadow_helper->registerWidget(widget);

    if (qobject_cast<QTabWidget*>(widget)) {
        //FIXME: unpolish, extensiable.
        m_tab_animation_helper->registerWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
        m_scrollbar_animation_helper->registerWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    /*!
      \todo QDateTimeEdit widget affected with calendarPopup() draw QComboBox style or QSpinBox style.
       So temporarily haven't added the QDateTimeEdit animation and style.
      */
    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->registerWidget(widget);
        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QSpinBox*>(widget) || qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if (widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(true);
        label->setScaledContents(true);
    }

    if (qobject_cast<QMessageBox *>(widget)) {
        widget->setAutoFillBackground(true);
        widget->setBackgroundRole(QPalette::Base);
    }

    if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QTabBar *>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    widget->installEventFilter(this);
}

void Qt5UKUIStyle::unpolish(QWidget *widget)
{
    m_shadow_helper->unregisterWidget(widget);

    widget->removeEventFilter(this);

    if (widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(false);
    }

    if (qobject_cast<QTabWidget*>(widget)) {
        m_tab_animation_helper->unregisterWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
        m_scrollbar_animation_helper->unregisterWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->unregisterWidget(widget);
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QSpinBox*>(widget) || qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if (qobject_cast<QLineEdit *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    Style::unpolish(widget);
}



QIcon Qt5UKUIStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    if (QApplication::desktopSettingsAware() && !QIcon::themeName().isEmpty()) {
        switch (standardIcon) {
        case SP_FileDialogListView:
            return QIcon::fromTheme(QLatin1String("view-list-symbolic"));
        case SP_FileDialogDetailedView:
            return QIcon::fromTheme(QLatin1String("view-grid-symbolic"));

        default:
            break;
        }
    }
    return Style::standardIcon(standardIcon, option, widget);
}



static inline uint qt_intensity(uint r, uint g, uint b)
{
    // 30% red, 59% green, 11% blue
    return (77 * r + 150 * g + 28 * b) / 255;
}

/*! \reimp */
QPixmap Qt5UKUIStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    switch (iconMode) {
    case QIcon::Disabled: {
        QStyleOption tmp = *option;
        tmp.state = State_Enabled;
        QPixmap target = HighLightEffect::ordinaryGeneratePixmap(pixmap, &tmp);
        QImage im = target.toImage().convertToFormat(QImage::Format_ARGB32);

        // Create a colortable based on the background (black -> bg -> white)
        QColor bg = option->palette.color(QPalette::Disabled, QPalette::Window);
        int red = bg.red();
        int green = bg.green();
        int blue = bg.blue();
        uchar reds[256], greens[256], blues[256];
        for (int i=0; i<128; ++i) {
            reds[i]   = uchar((red   * (i<<1)) >> 8);
            greens[i] = uchar((green * (i<<1)) >> 8);
            blues[i]  = uchar((blue  * (i<<1)) >> 8);
        }
        for (int i=0; i<128; ++i) {
            reds[i+128]   = uchar(qMin(red   + (i << 1), 255));
            greens[i+128] = uchar(qMin(green + (i << 1), 255));
            blues[i+128]  = uchar(qMin(blue  + (i << 1), 255));
        }

        int intensity = qt_intensity(red, green, blue);
        const int factor = 191;

        // High intensity colors needs dark shifting in the color table, while
        // low intensity colors needs light shifting. This is to increase the
        // perceived contrast.
        if ((red - factor > green && red - factor > blue)
            || (green - factor > red && green - factor > blue)
            || (blue - factor > red && blue - factor > green))
            intensity = qMin(255, intensity + 91);
//        else if (intensity <= 128)
//            intensity -= 51;

        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                // Calculate color table index, taking intensity adjustment
                // and a magic offset into account.
                uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));
                *scanLine = qRgba(reds[ci], greens[ci], blues[ci], qAlpha(pixel));
                ++scanLine;
            }
        }

        return QPixmap::fromImage(im);
    }

    default:
        break;
    }

    return Style::generatedIconPixmap(iconMode, pixmap, option);
}



void Qt5UKUIStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
    {
        return drawMenuPrimitive(option, painter, widget);
    }
    case PE_FrameFocusRect: {
        if (qobject_cast<const QAbstractItemView *>(widget))
            return;
        break;
    }

    case PE_Frame:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(option->palette.color(QPalette::Normal, QPalette::Dark), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(option->rect,6,6);
        painter->restore();
        return;
    }

    case PE_PanelButtonCommand:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto animator = m_button_animation_helper->animator(widget);
            const bool enable = button->state & State_Enabled;
            const bool hover = button->state & State_MouseOver;
            const bool sunken = button->state & State_Sunken;
            const bool on = button->state & State_On;
            qreal x_Radius = 4;
            qreal y_Radius = 4;
            bool isWindowButton = false;
            bool isWindowColoseButton = false;
            bool isImportant = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("isImportant").isValid())
                    isImportant = widget->property("isImportant").toBool();

                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();

                if (qobject_cast<const QComboBox*>(widget) || qobject_cast<const QLineEdit*>(widget)
                        || qobject_cast<const QAbstractSpinBox*>(widget))
                    useButtonPalette = true;
            }

            if (!enable) {
                if (animator) {
                    animator->stopAnimator("SunKen");
                    animator->stopAnimator("MouseOver");
                }
                painter->save();
                painter->setPen(Qt::NoPen);
                if (on)
                    painter->setBrush(button_DisableChecked());
                else if (button->features & QStyleOptionButton::Flat)
                    painter->setBrush(Qt::NoBrush);
                else
                    painter->setBrush(option->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (!(button->features & QStyleOptionButton::Flat)) {
                painter->save();
                painter->setPen(Qt::NoPen);
                if (isImportant)
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Highlight));
                else
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
            }

            if (animator == nullptr) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                if (sunken || on) {
                    if (isWindowColoseButton) {
                        painter->setBrush(QColor("#E44C50"));
                    } else if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                        QColor color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.15);
                        painter->setBrush(color);
                    } else {
                        if (isImportant)
                            painter->setBrush(highLight_Click());
                        else if (useButtonPalette || isWindowButton)
                            painter->setBrush(button_Click());
                        else
                            painter->setBrush(highLight_Click());
                    }
                } else if (hover) {
                    if (isWindowColoseButton) {
                        painter->setBrush(QColor("#F86458"));
                    } else if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                        QColor color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.1);
                        painter->setBrush(color);
                    } else {
                        if (isImportant)
                            painter->setBrush(highLight_Hover());
                        else if (useButtonPalette || isWindowButton)
                            painter->setBrush(button_Hover());
                        else
                            painter->setBrush(highLight_Hover());
                    }
                }
                painter->drawRoundedRect(button->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (sunken || on || animator->isRunning("SunKen") || animator->value("SunKen") == 1.0) {
                double opacity = animator->value("SunKen").toDouble();
                if (sunken || on) {
                    if (opacity == 0.0) {
                        animator->setAnimatorDirectionForward("SunKen", true);
                        animator->startAnimator("SunKen");
                    }
                } else {
                    if (opacity == 1.0) {
                        animator->setAnimatorDirectionForward("SunKen",false);
                        animator->startAnimator("SunKen");
                    }
                }

                QColor hoverColor, sunkenColor;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing,true);
                if (isWindowColoseButton) {
                    hoverColor = QColor("#F86458");
                    sunkenColor = QColor("#E44C50");
                } else if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                    hoverColor = option->palette.color(QPalette::Active, QPalette::Base);
                    hoverColor.setAlphaF(0.1);
                    sunkenColor = option->palette.color(QPalette::Active, QPalette::Base);
                    sunkenColor.setAlphaF(0.15);
                } else {
                    if (isImportant) {
                        hoverColor = highLight_Hover();
                        sunkenColor = highLight_Click();
                    } else if (useButtonPalette || isWindowButton) {
                        hoverColor = button_Hover();
                        sunkenColor = button_Click();
                    } else {
                        hoverColor = highLight_Hover();
                        sunkenColor = highLight_Click();
                    }
                }
                painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }

            if (hover || animator->isRunning("MouseOver")
                    || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
                double opacity = animator->value("MouseOver").toDouble();
                if (hover) {
                    animator->setAnimatorDirectionForward("MouseOver",true);
                    if (opacity == 0.0) {
                        animator->startAnimator("MouseOver");
                    }
                } else {
                    animator->setAnimatorDirectionForward("MouseOver",false);
                    if (opacity == 1.0) {
                        animator->startAnimator("MouseOver");
                    }
                }

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setOpacity(opacity);
                if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else {
                    if (isImportant)
                        painter->setBrush(highLight_Hover());
                    else if (isWindowButton && useDefaultPalette().contains(qAppName()))
                        painter->setBrush(button_Hover());
                    else if (useButtonPalette || isWindowButton)
                        painter->setBrush(button_Hover());
                    else
                        painter->setBrush(highLight_Hover());
                }
                painter->drawRoundedRect(option->rect, x_Radius, y_Radius);
                painter->restore();
                return;
            }
            return;
        }
        break;
    }

    case PE_PanelTipLabel://UKUI Tip  style: Open ground glass
        {
            if (widget && widget->isEnabled()) {
                QStyleOption opt = *option;

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QPainterPath rectPath;
                rectPath.addRoundedRect(option->rect.adjusted(+3,+3,-3,-3), 4, 4);

                // Draw a black floor
                QPixmap pixmap(option->rect.size());
                pixmap.fill(Qt::transparent);
                QPainter pixmapPainter(&pixmap);
                pixmapPainter.setRenderHint(QPainter::Antialiasing);
                pixmapPainter.setPen(Qt::transparent);
                pixmapPainter.setBrush(Qt::black);
                pixmapPainter.drawPath(rectPath);
                pixmapPainter.end();

                // Blur the black background
                QImage img = pixmap.toImage();
                qt_blurImage(img, 4, false, false);

                // Dig out the center part
                pixmap = QPixmap::fromImage(img);
                QPainter pixmapPainter2(&pixmap);
                pixmapPainter2.setRenderHint(QPainter::Antialiasing);
                pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
                pixmapPainter2.setPen(Qt::transparent);
                pixmapPainter2.setBrush(Qt::transparent);
                pixmapPainter2.drawPath(rectPath);

                // Shadow rendering
                painter->drawPixmap(option->rect, pixmap, pixmap.rect());

                //This is the beginning of drawing the bottom of the prompt box
                auto color = opt.palette.color(QPalette::ToolTipBase);
                if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
                    auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
                    color.setAlphaF(opacity);
                }

                if (qApp->property("blurEnable").isValid()) {
                    bool blurEnable = qApp->property("blurEnable").toBool();
                    if (!blurEnable) {
                        color.setAlphaF(1);
                    }
                }

                opt.palette.setColor(QPalette::ToolTipBase, color);
                painter->setRenderHint(QPainter::Antialiasing);
                QPen pen(opt.palette.toolTipBase().color().darker(150), 1);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(Qt::transparent);
                painter->setBrush(color);

                QPainterPath path;
                auto region = widget->mask();
                if (region.isEmpty()) {
                    path.addRoundedRect(opt.rect.adjusted(+3,+3,-3,-3), 4, 4);
                } else {
                    path.addRegion(region);
                }

                painter->drawPath(path);
                painter->restore();
                return;
            }
            return Style::drawPrimitive(element, option, painter, widget);
        }

    case PE_FrameStatusBar://UKUI Status style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Dark));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_IndicatorButtonDropDown: //UKUI IndicatorButton  style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        if(widget && widget->isEnabled()){
            if (option->state & State_MouseOver) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->drawRoundedRect(option->rect.left()-4,option->rect.y(),option->rect.width()+4,option->rect.height(),4,4);
            }
        }
        painter->restore();
        return;
    }

    case PE_PanelButtonTool:
    {
        auto animator = m_button_animation_helper->animator(widget);

        bool isWindowColoseButton = false;
        bool isWindowButton = false;
        bool useButtonPalette = false;
        if (widget) {
            if (widget->property("isWindowButton").isValid()) {
                if (widget->property("isWindowButton").toInt() == 0x01)
                    isWindowButton = true;
                if (widget->property("isWindowButton").toInt() == 0x02)
                    isWindowColoseButton = true;
            }
            if (widget->property("useButtonPalette").isValid())
                useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        const bool enable = option->state & State_Enabled;
        const bool raise = option->state & State_AutoRaise;
        const bool sunken = option->state & State_Sunken;
        const bool hover = option->state & State_MouseOver;
        const bool on = option->state & State_On;

        if (!enable) {
            if (animator) {
                animator->stopAnimator("SunKen");
                animator->stopAnimator("MouseOver");
            }
            painter->save();
            painter->setPen(Qt::NoPen);
            if (on)
                painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Button));
            else if (raise)
                painter->setBrush(Qt::NoBrush);
            else
                painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (!raise) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Active, QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
        }

        if (animator == nullptr) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            if (sunken || on) {
                if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.15);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#E44C50"));
                } else if (isWindowButton || useButtonPalette) {
                    painter->setBrush(button_Click());
                } else {
                    painter->setBrush(highLight_Click());
                }
            } else if (hover) {
                if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                    QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else if (isWindowButton || useButtonPalette) {
                    painter->setBrush(button_Hover());
                } else {
                    painter->setBrush(highLight_Hover());
                }
            }
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (sunken || on || animator->isRunning("SunKen")
                || animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
            double opacity = animator->value("SunKen").toDouble();
            if (sunken || on) {
                if (opacity == 0.0) {
                    animator->setAnimatorDirectionForward("SunKen",true);
                    animator->startAnimator("SunKen");
                }
            } else {
                if (animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
                    animator->setAnimatorDirectionForward("SunKen",false);
                    animator->startAnimator("SunKen");
                }
            }
            QColor hoverColor, sunkenColor;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing,true);
            if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                hoverColor = option->palette.color(QPalette::Active, QPalette::Base);
                hoverColor.setAlphaF(0.1);
                sunkenColor = option->palette.color(QPalette::Active, QPalette::Base);
                sunkenColor.setAlphaF(0.15);
            } else if (isWindowColoseButton) {
                hoverColor = QColor("#F86458");
                sunkenColor = QColor("#E44C50");
            } else if (isWindowButton || useButtonPalette){
                hoverColor = button_Hover();
                sunkenColor = button_Click();
            } else {
                hoverColor = highLight_Hover();
                sunkenColor = highLight_Click();
            }
            painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (hover || animator->isRunning("MouseOver")
                || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
            double opacity = animator->value("MouseOver").toDouble();
            if (option->state & State_MouseOver) {
                animator->setAnimatorDirectionForward("MouseOver",true);
                if(opacity == 0.0) {
                    animator->startAnimator("MouseOver");
                }
            } else {
                animator->setAnimatorDirectionForward("MouseOver",false);
                if (opacity == 1.0) {
                    animator->startAnimator("MouseOver");
                }
            }
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            painter->setOpacity(opacity);
            if (isWindowButton && useDefaultPalette().contains(qAppName())) {
                QColor color = option->palette.color(QPalette::Active, QPalette::Base);
                color.setAlphaF(0.1);
                painter->setBrush(color);
            } else if (isWindowColoseButton) {
                painter->setBrush(QColor("#F86458"));
            } else if (isWindowButton || useButtonPalette){
                painter->setBrush(button_Hover());
            } else {
                painter->setBrush(highLight_Hover());
            }
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }
        return;
    }

    case PE_IndicatorTabClose:
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Text));
        if (option->state & (State_On | State_Sunken)) {
            painter->setOpacity(0.15);
        }
        else if (option->state & State_MouseOver) {
            painter->setOpacity(0.1);
        }
        else
            painter->setOpacity(0.0);
        painter->drawEllipse(option->rect);
        painter->restore();
        QIcon icon = QIcon::fromTheme("window-close-symbolic");
        if (!icon.isNull()) {
            int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
            QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize), QIcon::Normal, QIcon::On);
            pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
        }
        return;
    }

    case PE_FrameTabBarBase:
    {
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(tbb->palette.brush(QPalette::Active, QPalette::Window));
            painter->drawRect(tbb->tabBarRect);
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameTabWidget: {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(twf->palette.brush(QPalette::Active, QPalette::Base));
            painter->drawRect(option->rect.adjusted(0, 0, 0, 0));
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameGroupBox: //UKUI GroupBox style:
    {
        /*
        * Remove the style of the bounding box according to the design
        */
        //painter->save();
        //painter->setRenderHint(QPainter::Antialiasing,true);
        //painter->setPen(option->palette.color(QPalette::Base));
        //painter->setBrush(option->palette.color(QPalette::Base));
        //painter->drawRoundedRect(option->rect,4,4);
        //painter->restore();
        return;
    }

    case PE_PanelLineEdit://UKUI Text edit style
    {
        if (widget) {
            if (QAction *clearAction = widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"))) {
                QStyleOption subOption = *option;
                QColor color = subOption.palette.color(QPalette::Text);
                color.setAlphaF(1.0);
                subOption.palette.setColor(QPalette::Text, color);
                clearAction->setIcon(QIcon(HighLightEffect::ordinaryGeneratePixmap(clearAction->icon().pixmap(16, 16), &subOption, widget, HighLightEffect::BothDefaultAndHighlit)));
            }
        }

        // Conflict with qspinbox and so on, The widget text cannot use this style
        if (widget) {
            if (widget->parentWidget())
                if (widget->parentWidget()->inherits("QDoubleSpinBox")|| widget->parentWidget()->inherits("QSpinBox")
                        || widget->parentWidget()->inherits("QComboBox") || widget->parentWidget()->inherits("QDateTimeEdit"))
                {
                    return;
                }
        }

        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            const bool enable = f->state & State_Enabled;
            const bool focus = f->state & State_HasFocus;

            if (!enable) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (f->state & State_ReadOnly) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Active, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (focus) {
                painter->save();
                painter->setPen(QPen(f->palette.brush(QPalette::Active, QPalette::Highlight),
                                     2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                painter->restore();
            } else {
                QStyleOptionButton button;
                button.state = option->state & ~(State_Sunken | State_On);
                button.rect = option->rect;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);

                if (f->state & State_MouseOver) {
                    QRectF rect = f->rect;
                    painter->save();
                    painter->setPen(QPen(f->palette.brush(QPalette::Active, QPalette::Highlight),
                                         1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                    painter->restore();
                }
            }
            return;
        }
        break;
    }

    case PE_IndicatorArrowUp:
    {
        QIcon icon = QIcon::fromTheme("ukui-up-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowDown:
    {
        QIcon icon = QIcon::fromTheme("ukui-down-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowRight:
    {
        QIcon icon = QIcon::fromTheme("ukui-end-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowLeft:
    {
        QIcon icon = QIcon::fromTheme("ukui-start-symbolic");
        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorRadioButton:
    {
        if (const QStyleOptionButton* radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            QRectF rect = radiobutton->rect.adjusted(1, 1, -1, -1);

            const bool useDarkPalette = !useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool()
                                                                                      || (m_is_default_style && specialList().contains(qAppName())));
            bool enable = radiobutton->state & State_Enabled;
            bool mouseOver = radiobutton->state & State_MouseOver;
            bool sunKen = radiobutton->state & State_Sunken;
            bool On = radiobutton->state & State_On;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            if (On) {
                if (enable) {
                    if (sunKen) {
                        painter->setPen(QColor(25, 101, 207));
                        painter->setBrush(highLight_Click());
                    } else if (mouseOver) {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(highLight_Hover());
                    } else {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(radiobutton->palette.brush(QPalette::Active, QPalette::Highlight));
                    }
                    painter->drawEllipse(rect);
                    QRectF childRect(rect.x(), rect.y(), 6, 6);
                    childRect.moveCenter(rect.center());
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(radiobutton->palette.brush(QPalette::Active, QPalette::HighlightedText));
                    painter->drawEllipse(childRect);
                } else {
                    if (useDarkPalette) {
                        painter->setPen(QColor(48, 48, 51));
                        painter->setBrush(QColor(28, 28, 30));
                    } else {
                        painter->setPen(QColor(224, 224, 224));
                        painter->setBrush(QColor(233, 233, 233));
                    }
                    painter->drawEllipse(rect);
                    QRectF childRect(rect.x(), rect.y(), 6, 6);
                    childRect.moveCenter(rect.center());
                    painter->setBrush(radiobutton->palette.brush(QPalette::Disabled, QPalette::ButtonText));
                    painter->drawEllipse(childRect);
                }
            } else {
                if (enable) {
                    if (sunKen) {
                        if (useDarkPalette) {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(6, 35, 97));
                        } else {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(179, 221, 255));
                        }
                    } else if (mouseOver) {
                        if (useDarkPalette) {
                            painter->setPen(QColor(55, 144, 250));
                            painter->setBrush(QColor(9, 53, 153));
                        } else {
                            painter->setPen(QColor(97, 173, 255));
                            painter->setBrush(QColor(219, 240, 255));
                        }
                    } else {
                        if (useDarkPalette) {
                            painter->setPen(QColor(72, 72, 77));
                            painter->setBrush(QColor(48, 48, 51));
                        } else {
                            painter->setPen(QColor(191, 191, 191));
                            painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Window));
                        }
                    }
                } else {
                    if (useDarkPalette) {
                        painter->setPen(QColor(48, 48, 51));
                        painter->setBrush(QColor(28, 28, 30));
                    } else {
                        painter->setPen(QColor(224, 224, 224));
                        painter->setBrush(QColor(233, 233, 233));
                    }
                }
                painter->drawEllipse(rect);
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorCheckBox:
    {
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            const bool useDarkPalette = !useDefaultPalette().contains(qAppName()) && (qApp->property("preferDark").toBool()
                                                                                      || (m_is_default_style && specialList().contains(qAppName())));
            bool enable = checkbox->state & State_Enabled;
            bool mouseOver = checkbox->state & State_MouseOver;
            bool sunKen = checkbox->state & State_Sunken;
            bool on = checkbox->state & State_On;
            bool noChange = checkbox->state & State_NoChange;

            QRectF rect = checkbox->rect;
            int width = rect.width();
            int heigth = rect.height();
            int x_Radius = 4;
            int y_Radius = 4;

            QPainterPath path;
            if (on) {
                path.moveTo(width/4 + checkbox->rect.left(), heigth/2 + checkbox->rect.top());
                path.lineTo(width*0.45 + checkbox->rect.left(), heigth*3/4 + checkbox->rect.top());
                path.lineTo(width*3/4 + checkbox->rect.left(), heigth/4 + checkbox->rect.top());
            } else if (noChange){
                path.moveTo(rect.left() + width/4, rect.center().y());
                path.lineTo(rect.right() - width/4 , rect.center().y());
            }

            painter->save();
            painter->setClipRect(rect);
            painter->setRenderHint(QPainter::Antialiasing,true);
            if (enable) {
                if (on | noChange) {
                    if (sunKen) {
                        painter->setPen(QColor(25, 101, 207));
                        painter->setBrush(highLight_Click());
                    } else if (mouseOver) {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(highLight_Hover());
                    } else {
                        painter->setPen(QColor(36, 109, 212));
                        painter->setBrush(checkbox->palette.brush(QPalette::Active, QPalette::Highlight));
                    }
                    painter->drawRoundedRect(rect, x_Radius, y_Radius);

                    painter->setPen(QPen(checkbox->palette.brush(QPalette::Active, QPalette::HighlightedText), 2,
                                         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                } else {
                    if (sunKen) {
                        if (useDarkPalette) {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(6, 35, 97));
                        } else {
                            painter->setPen(QColor(36, 109, 212));
                            painter->setBrush(QColor(179, 221, 255));
                        }
                    } else if (mouseOver) {
                        if (useDarkPalette) {
                            painter->setPen(QColor(55, 144, 250));
                            painter->setBrush(QColor(9, 53, 153));
                        } else {
                            painter->setPen(QColor(97, 173, 255));
                            painter->setBrush(QColor(219, 240, 255));
                        }
                    } else {
                        if (useDarkPalette) {
                            painter->setPen(QColor(72, 72, 77));
                            painter->setBrush(QColor(48, 48, 51));
                        } else {
                            painter->setPen(QColor(191, 191, 191));
                            painter->setBrush(checkbox->palette.color(QPalette::Active, QPalette::Window));
                        }
                    }
                    painter->drawRoundedRect(rect, x_Radius, y_Radius);
                }
            } else {
                if (useDarkPalette) {
                    painter->setPen(QColor(48, 48, 51));
                    painter->setBrush(QColor(28, 28, 30));
                } else {
                    painter->setPen(QColor(224, 224, 224));
                    painter->setBrush(QColor(233, 233, 233));
                }
                painter->drawRoundedRect(rect, x_Radius, y_Radius);
                if (on | noChange) {
                    painter->setPen(QPen(checkbox->palette.brush(QPalette::Disabled, QPalette::ButtonText), 2,
                                         Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorTabTearLeft:
        return;

    case PE_IndicatorTabTearRight:
        return;

    case PE_PanelScrollAreaCorner:
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_IndicatorHeaderArrow:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            if (header->sortIndicator == QStyleOptionHeader::SortDown)
                return proxy()->drawPrimitive(PE_IndicatorArrowDown, header, painter, widget);
            if (header->sortIndicator == QStyleOptionHeader::SortUp)
                return proxy()->drawPrimitive(PE_IndicatorArrowUp, header, painter, widget);
        }
        break;
    }

    case PE_PanelItemViewRow:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QPalette::ColorGroup cg = (widget ? widget->isEnabled() : (vi->state & QStyle::State_Enabled))
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(vi->state & QStyle::State_Active))
                cg = QPalette::Inactive;

            int Radius = 4;
            bool isTree = false;
            if (qobject_cast<const QTreeView*>(widget))
                isTree = true;
            if (isTree) {
                if (proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget)) {
                    painter->save();
                    painter->setPen(Qt::NoPen);
                    if (vi->state & State_Selected) {
                        painter->setBrush(vi->palette.brush(QPalette::Active, QPalette::Highlight));
                        painter->drawRect(vi->rect);
                    } else if (vi->state & State_Sunken) {
                        painter->setBrush(highLight_Click());
                        painter->drawRect(vi->rect);
                    } else if (vi->state & State_MouseOver) {
                        painter->setBrush(highLight_Hover());
                        painter->drawRect(vi->rect);
                    } else if (vi->features & QStyleOptionViewItem::Alternate) {
                        painter->setBrush(vi->palette.brush(cg, QPalette::AlternateBase));
                        painter->drawRect(vi->rect);
                    }
                    painter->restore();
                }
            } else if (vi->features & QStyleOptionViewItem::Alternate) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(vi->palette.brush(cg, QPalette::AlternateBase));
                painter->drawRoundedRect(vi->rect, Radius, Radius);
            }
            return;
        }
        break;
    }

    case PE_PanelItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            bool isIconMode = false;
            const bool enable = vi->state & State_Enabled;
            const bool select = vi->state & State_Selected;
            const bool hover = vi->state & State_MouseOver;
            const bool sunken = vi->state & State_Sunken;
            int iconMode_Radius = 6;
            int Radius = 4;

            if (!enable)
                return;

            if ((vi->decorationPosition == QStyleOptionViewItem::Top) || (vi->decorationPosition ==  QStyleOptionViewItem::Bottom))
                isIconMode = true;

            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, option, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, option, widget);

            if (isIconMode) {
                if (select) {
                    int Margin_Height = 2;
                    const int Margin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, option, widget) + 1;
                    iconRect.setRect(option->rect.x(), option->rect.y(), option->rect.width(), iconRect.height() + Margin + Margin_Height);
                    textRect.setRect(option->rect.x(), iconRect.bottom() + 1, option->rect.width(), textRect.height() + Margin_Height);
                    QPainterPath iconPath, textPath;
                    iconPath.moveTo(iconRect.x(), iconRect.y() + iconMode_Radius);
                    iconPath.arcTo(iconRect.x(), iconRect.y(), iconMode_Radius * 2, iconMode_Radius * 2, 180, -90);
                    iconPath.lineTo(iconRect.right() + 1 - iconMode_Radius, iconRect.y());
                    iconPath.arcTo(iconRect.right() + 1 - iconMode_Radius * 2, iconRect.y(), iconMode_Radius * 2, iconMode_Radius * 2,
                                   90, -90);
                    iconPath.lineTo(iconRect.right() + 1, iconRect.bottom() + 1);
                    iconPath.lineTo(iconRect.left(), iconRect.bottom() + 1);
                    iconPath.lineTo(iconRect.left(), iconRect.top() + iconMode_Radius);

                    textPath.moveTo(textRect.x(), textRect.y());
                    textPath.lineTo(textRect.right() + 1, textRect.y());
                    textPath.lineTo(textRect.right() + 1, textRect.bottom() + 1 - iconMode_Radius);
                    textPath.arcTo(textRect.right() + 1 - iconMode_Radius * 2, textRect.bottom() + 1 - iconMode_Radius * 2, iconMode_Radius * 2, iconMode_Radius * 2,
                                   0, -90);
                    textPath.lineTo(textRect.left() + iconMode_Radius, textRect.bottom() + 1);
                    textPath.arcTo(textRect.left(), textRect.bottom() + 1 - iconMode_Radius * 2, iconMode_Radius * 2, iconMode_Radius * 2,
                                   270, -90);
                    textPath.lineTo(textRect.left(), textRect.top());

                    painter->save();
                    painter->setPen(Qt::NoPen);
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    if (sunken) {
                        painter->setBrush(button_Click());
                        painter->drawPath(iconPath);
                        painter->setBrush(highLight_Click());
                        painter->drawPath(textPath);
                    } else if (hover) {
                        painter->setBrush(button_Hover());
                        painter->drawPath(iconPath);
                        painter->setBrush(highLight_Hover());
                        painter->drawPath(textPath);
                    } else {
                        painter->setBrush(vi->palette.brush(QPalette::Active, QPalette::Button));
                        painter->drawPath(iconPath);
                        painter->setBrush(vi->palette.brush(QPalette::Active, QPalette::Highlight));
                        painter->drawPath(textPath);
                    }
                    painter->restore();
                } else {
                    painter->save();
                    painter->setPen(Qt::NoPen);
                    if (sunken) {
                        painter->setBrush(button_Click());
                        painter->drawRoundedRect(option->rect, iconMode_Radius, iconMode_Radius);
                    } else if (hover) {
                        painter->setBrush(button_Hover());
                        painter->drawRoundedRect(option->rect, iconMode_Radius, iconMode_Radius);
                    }
                    painter->restore();
                }
            } else {
                if (vi->backgroundBrush.style() != Qt::NoBrush) {
                    QPointF oldBO = painter->brushOrigin();
                    painter->setBrushOrigin(vi->rect.topLeft());
                    painter->fillRect(vi->rect, vi->backgroundBrush);
                    painter->setBrushOrigin(oldBO);
                }
                painter->save();
                painter->setPen(Qt::NoPen);
                bool isTree = false;
                if (qobject_cast<const QTreeView*>(widget))
                    isTree = true;
                QPainterPath path;
                if (isTree) {
                    path.addRect(vi->rect);
                } else {
                    path.addRoundedRect(vi->rect, Radius, Radius);
                }
                if (select) {
                    painter->setBrush(vi->palette.brush(QPalette::Active, QPalette::Highlight));
                    painter->drawPath(path);
                } else if (sunken) {
                    painter->setBrush(highLight_Click());
                    painter->drawPath(path);
                } else if (hover) {
                    painter->setBrush(highLight_Hover());
                    painter->drawPath(path);
                }
                painter->restore();
            }
            if (vi->state & State_HasFocus) {

            }
            return;
        }
        break;
    }

    case PE_IndicatorBranch:
    {
        if (!(option->state & State_Children)) {
            return;
        }
        QStyleOption subOption = *option;
        if (proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget))
            subOption.state = option->state;
        else
            subOption.state = option->state & State_Enabled ? State_Enabled : State_None;

        if (option->state & State_Open) {
            proxy()->drawPrimitive(PE_IndicatorArrowDown, &subOption, painter, widget);
        } else {
            const bool reverse = (option->direction == Qt::RightToLeft);
            proxy()->drawPrimitive(reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight, &subOption, painter, widget);
        }
        return;
    }

    case PE_IndicatorViewItemCheck:
        return proxy()->drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);

    case PE_IndicatorItemViewItemDrop:
    {
        QRect rect = option->rect;
        int Radius = 4;
        painter->save();
        painter->setClipRect(rect);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(option->palette.color(QPalette::Active, QPalette::Highlight));
        painter->setBrush(Qt::NoBrush);
        if (option->rect.height() == 0)
            painter->drawLine(rect.topLeft(), rect.topRight());
        else
            painter->drawRoundedRect(rect, Radius, Radius);
        painter->restore();
        return;
    }

    default:
        break;
    }

    return Style::drawPrimitive(element, option, painter, widget);
}

void Qt5UKUIStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            if (widget && widget->property("drawScrollBarGroove").isValid() && (!widget->property("drawScrollBarGroove").toBool())) {

            } else {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(bar->palette.brush(QPalette::Active, QPalette::Base));
                painter->drawRect(bar->rect);
                painter->restore();
            }
            QStyleOptionSlider newScrollbar = *bar;
            newScrollbar.rect = proxy()->subControlRect(control, option, SC_ScrollBarSlider, widget);
//            if (!(bar->activeSubControls & SC_ScrollBarSlider)) {
//                newScrollbar.state &= ~(State_Sunken | State_MouseOver);
//            }
            proxy()->drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);
            return;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            const bool enable = comboBox->state & State_Enabled;
            const bool on = comboBox->state & State_On;
            const bool hover = comboBox->state & State_MouseOver;

            if (!enable) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(comboBox->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (comboBox->editable) {
                painter->save();
                if (comboBox->state & (State_HasFocus | State_On)) {
                    painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                } else {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Button));
                }
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                painter->restore();
            } else {
                QStyleOptionButton button;
                button.state = option->state;
                button.rect = option->rect;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);
                if (on) {
                    painter->save();
                    painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(Qt::NoBrush);
                    painter->setRenderHint(QPainter::Antialiasing,true);
                    painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                    painter->restore();
                }
            }

            if (hover) {
                QRectF rect = comboBox->rect;
                painter->save();
                painter->setPen(QPen(comboBox->palette.brush(QPalette::Active, QPalette::Highlight),
                                     1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter->setBrush(Qt::NoBrush);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                painter->restore();
            }

            return;
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            bool enable = sb->state & State_Enabled;
            bool hover = enable && (sb->state & State_MouseOver);
            bool up = sb->activeSubControls == SC_SpinBoxUp;
            bool down = sb->activeSubControls == SC_SpinBoxDown;
            bool focus = sb->state & State_HasFocus;
            QRect upRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
            QRect downRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);

            QStyleOption upOption, downOption;
            upOption.rect = upRect;
            downOption.rect = downRect;
            if (!enable) {
                upOption.state = State_None;
                downOption.state = State_None;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(sb->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
            } else if (sb->stepEnabled == QAbstractSpinBox::StepNone) {
                upOption.state = State_Enabled;
                downOption.state = State_Enabled;
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Button));
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
            } else {
                upOption.state |= State_Enabled;
                downOption.state |= State_Enabled;
                if (focus) {
                    painter->save();
                    painter->setPen(QPen(sb->palette.brush(QPalette::Active, QPalette::Highlight),
                                         2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), 4, 4);
                    painter->restore();
                } else {
                    QStyleOptionButton button;
                    button.state = option->state & ~(State_Sunken | State_On);
                    button.rect = option->rect;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &button, painter, widget);

                    if (hover) {
                        QRectF rect = sb->rect;
                        painter->save();
                        painter->setPen(QPen(sb->palette.brush(QPalette::Active, QPalette::Highlight),
                                             1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->setBrush(Qt::NoBrush);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
                        painter->restore();
                    }
                }
            }

            int radius = 4;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::NoBrush);
            if (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown)) {
                if (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) {
                    QPainterPath upPath;
                    upPath.moveTo(upRect.left(), upRect.top() + radius);
                    upPath.arcTo(upRect.left(), upRect.top(), radius * 2, radius * 2, 180, -90);
                    upPath.lineTo(upRect.right() - radius, upRect.top());
                    upPath.arcTo(upRect.right() + 1 - 2 * radius, upRect.top(), radius * 2, radius * 2, 90, -90);
                    upPath.lineTo(upRect.right() + 1, upRect.bottom());
                    upPath.lineTo(upRect.left(), upRect.bottom());
                    upPath.lineTo(upRect.left(), upRect.top() - radius);
                    upOption.state |= State_Enabled;
                    if (up) {
                        painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                        upOption.state |= State_MouseOver;
                        if (option->state & State_Sunken) {
                            painter->setBrush(highLight_Click());
                            upOption.state |= State_Sunken;
                        }
                        painter->drawPath(upPath);
                    }
                } else {
                    upOption.state = State_None;
                }

                if (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) {
                    QPainterPath downPath;
                    downPath.moveTo(downRect.left(), downRect.bottom() - radius);
                    downPath.arcTo(downRect.left(), downRect.bottom() - radius * 2, radius * 2, radius * 2, 180, 90);
                    downPath.lineTo(downRect.right() - radius, downRect.bottom());
                    downPath.arcTo(downRect.right() + 1 - radius * 2, downRect.bottom() - radius * 2, radius * 2, radius * 2, 270, 90);
                    downPath.lineTo(downRect.right() + 1, downRect.top());
                    downPath.lineTo(downRect.left(), downRect.top());
                    downPath.lineTo(downRect.left(), downRect.bottom() - radius);
                    downOption.state |= State_Enabled;
                    painter->setBrush(Qt::NoBrush);
                    painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                    if (down) {
                        painter->setBrush(sb->palette.brush(QPalette::Active, QPalette::Highlight));
                        downOption.state |= State_MouseOver;
                        if (option->state & State_Sunken) {
                            painter->setBrush(highLight_Click());
                            downOption.state |= State_Sunken;
                        }
                        painter->drawPath(downPath);
                    }
                } else {
                    downOption.state = State_None;
                }

                if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus) {

                } else if (sb->buttonSymbols == QAbstractSpinBox::UpDownArrows) {
                    proxy()->drawPrimitive(PE_IndicatorArrowUp, &upOption, painter, widget);
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &downOption, painter, widget);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

//    case CC_Slider :
//        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
//            //Size and location of each rectangle used
//            QRectF rect = option->rect;
//            QRectF rectHandle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
//            QRectF rectSliderTickmarks = proxy()->subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
//            QRect rectGroove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
//            const bool enable = option->state & State_Enabled;
//            const QColor highlight = option->palette.color(QPalette::Highlight);
//            const QColor dis_highlight = option->palette.color(QPalette::Disabled,QPalette::Button).darker(120);
//            QPen pen;
//            //Drawing chute (line)
//            if (option->subControls & SC_SliderGroove) {
//                pen.setStyle(Qt::CustomDashLine);
//                QVector<qreal> dashes;
//                //qreal space = 1.3;
//                qreal space = 0;
//                dashes << 0.1 << space;
//                // dashes << -0.1 << space;
//                pen.setDashPattern(dashes);
//                pen.setWidthF(3);
//                pen.setColor(enable ? highlight.lighter() : dis_highlight);
//                painter->setPen(pen);
//                painter->setRenderHint(QPainter::Antialiasing);

//                if (slider->orientation == Qt::Horizontal) {
//                    painter->drawLine(QPointF(rectGroove.left(), rectHandle.center().y()), QPointF(rectHandle.left(), rectHandle.center().y()));
//                    pen.setColor(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
//                    painter->setPen(pen);
//                    painter->drawLine(QPointF(rectGroove.right(), rectHandle.center().y()), QPointF(rectHandle.right(), rectHandle.center().y()));
//                } else {
//                    painter->drawLine(QPointF(rectGroove.center().x(), rectGroove.bottom()), QPointF(rectGroove.center().x(),  rectHandle.bottom()));
//                    pen.setColor(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
//                    painter->setPen(pen);
//                    painter->drawLine(QPointF(rectGroove.center().x(),  rectGroove.top()), QPointF(rectGroove.center().x(),  rectHandle.top()));
//                }
//            }

//            //Painting slider
//            if (option->subControls & SC_SliderHandle) {
//                pen.setStyle(Qt::SolidLine);
//                painter->setPen(Qt::NoPen);
//                painter->setBrush(enable ? highlight : dis_highlight);
//                painter->drawEllipse(rectHandle);
//            }

//            //Drawing scale
//            if ((option->subControls & SC_SliderTickmarks) && slider->tickInterval) {
//                painter->setPen(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::WindowText));
//                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, slider, widget);
//                int interval = slider->tickInterval;
//                //int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);
//                //int ticks = slider->tickPosition;
//                int v = slider->minimum;
//                int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
//                while (v <= slider->maximum + 1) {
//                    const int v_ = qMin(v, slider->maximum);
//                    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, v_, available) + len / 2;

//                    if (slider->orientation == Qt::Horizontal) {
//                        if (slider->tickPosition == QSlider::TicksBothSides) {
//                            painter->drawLine(pos, int(rect.top()), pos, int(rectHandle.top()));
//                            painter->drawLine(pos, int(rect.bottom()), pos, int(rectHandle.bottom()));
//                        } else {
//                            painter->drawLine(pos, int(rectSliderTickmarks.top()), pos, int(rectSliderTickmarks.bottom()));
//                        }
//                    } else {
//                        if (slider->tickPosition == QSlider::TicksBothSides) {
//                            painter->drawLine(int(rect.left()), pos, int(rectHandle.left()), pos);
//                            painter->drawLine(int(rect.right()), pos, int(rectHandle.right()), pos);
//                        } else {
//                            painter->drawLine(int(rectSliderTickmarks.left()), pos, int(rectSliderTickmarks.right()), pos);
//                        }
//                    }
//                    // in the case where maximum is max int
//                    int nextInterval = v + interval;
//                    if (nextInterval < v)
//                        break;
//                    v = nextInterval;
//                }
//            }
//            return;
//        }

    case CC_Slider:
    {
        if(const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            QRect groove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            const bool horizontal = slider->orientation == Qt::Horizontal;
            const bool enable = slider->state & State_Enabled;
            const bool ticksAbove = slider->tickPosition & QSlider::TicksAbove;
            const bool ticksBelow = slider->tickPosition & QSlider::TicksBelow;
            const QColor button = slider->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button);
            QColor hightligt = slider->palette.color(QPalette::Highlight).lighter();
            if(!enable)
            {
                hightligt = slider->palette.color(QPalette::Disabled,QPalette::Button).darker(120);
            }
            int Slider_GrooveMargin = 4;
            if (horizontal) {
                groove.adjust(0, (groove.height() - Slider_GrooveMargin) / 2, 0, -(groove.height() - Slider_GrooveMargin) /2);
            } else {
                groove.adjust((groove.width() - Slider_GrooveMargin) / 2, 0, -(groove.width() - Slider_GrooveMargin) / 2, 0);
            }

            if(slider->subControls & SC_SliderGroove)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                //draw blue groove highlight
                QRect hlRect;
                if(horizontal)
                {
                   if(slider->upsideDown)
                      hlRect.setRect(handle.right(),groove.top(),groove.right() - handle.center().x(),groove.height());
                   else
                      hlRect.setRect(groove.left(),groove.top(),handle.center().x() - groove.left(),groove.height());
                }
                else
                {
                   hlRect.setRect(groove.left(), handle.center().y(), groove.width(), groove.bottom() - handle.center().y());
                }
                painter->setPen(button);
                painter->setBrush(button);
                painter->drawRoundedRect(groove, 2, 2);
                painter->setPen(hightligt);
                painter->setBrush(hightligt);
                painter->drawRoundedRect(hlRect,2,2);
                painter->restore();
            }

            int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int sliderPos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,slider->sliderPosition,
                                                    (horizontal ? slider->rect.width(): slider->rect.height()) - len,slider->upsideDown);
            if(slider->subControls & SC_SliderTickmarks)
            {
                int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
                int tickSize = tickOffset;
//                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget);
                int interval = slider->tickInterval;
                if (interval <= 0)
                {
//                    interval = slider->singleStep;
//                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,available)
//                              - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,0, available) < 3)
                        interval = slider->pageStep;
                }
                if (interval <= 0)
                    interval = 1;
                int v = slider->minimum;
                int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
                while (v <= slider->maximum + 1)
                {
                   if (v == slider->maximum + 1 && interval == 1)
                       break;
                   const int v_ = qMin(v, slider->maximum);
                   int pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,v_, (horizontal ? slider->rect.width():
                                                     slider->rect.height()) - len,slider->upsideDown) + len / 2;
                   //int extra = 2 - ((v_ == slider->minimum || v_ == slider->maximum) ? 1 : 0);

                   painter->save();
                   painter->setPen(QPen(slider->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::WindowText),1));
                   painter->setBrush(Qt::NoBrush);
                   if (horizontal)
                   {
                       if(pos <= sliderPos + handle.width()/2)
                       {
                           painter->setPen(QPen(hightligt,1));
                       }
                       if (ticksAbove)
                       {
                           painter->drawLine(pos, groove.top() - tickOffset,pos, groove.top() - tickOffset - tickSize);
                       }
                       if (ticksBelow)
                       {
                           painter->drawLine(pos, groove.bottom() + tickOffset,pos, groove.bottom() + tickOffset + tickSize);
                       }
                   }
                   else
                   {
                       if(pos >= sliderPos + handle.width()/2)
                       {
                           painter->setPen(QPen(hightligt,1));
                       }
                       if (ticksAbove)
                       {
                           painter->drawLine(groove.left() - tickOffset, pos,groove.left() - tickOffset - tickSize, pos);
                       }
                       if (ticksBelow)
                       {
                           painter->drawLine(groove.right() + tickOffset, pos,groove.right() + tickOffset + tickSize, pos);
                       }
                   }
                   // in the case where maximum is max int
                   int nextInterval = v + interval;
                   if (nextInterval < v)
                       break;
                   v = nextInterval;
                   painter->restore();
                }
            }

            if(slider->subControls & SC_SliderHandle)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(slider->palette.color(QPalette::Highlight).lighter(125));
                if(slider->state & State_Sunken && slider->activeSubControls == SC_SliderHandle)
                {
                    painter->setBrush(slider->palette.color(QPalette::Highlight));
                }
                else if(slider->state & State_MouseOver && slider->activeSubControls == SC_SliderHandle)
                {
                    painter->setBrush(slider->palette.color(QPalette::Highlight).lighter());
                }
                if(!enable)
                {
                    painter->setBrush(hightligt);
                }
                painter->drawEllipse(handle);
                painter->restore();
            }
            return;
        }
        break;
    }

    case CC_Dial:
    {
        if(const QStyleOptionSlider* dial = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            const bool enable = dial->state & State_Enabled;
            int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
            int r = qMin(dial->rect.width(), dial->rect.height())/2;
            int r_ = r - tickOffset*2;
            QRect br(dial->rect.topLeft(), QSize(r_*2, r_*2));
            br.moveCenter(dial->rect.center());
            QColor highlight = enable ? dial->palette.color(QPalette::Highlight) : dial->palette.color(QPalette::Disabled, QPalette::Button).darker(120);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
            painter->setBrush(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
            painter->drawEllipse(br);

            int tickHandle = proxy()->pixelMetric(PM_SliderControlThickness, option, widget);
            int handle_r = tickHandle < r_/2 ? tickHandle : r_/2;
            int Circle_r = (r_ + handle_r)/2;
            qreal fist = QT5UKUISTYLE_H::calcRadialPos(dial, dial->minimum);
            qreal dp = QT5UKUISTYLE_H::calcRadialPos(dial, dial->sliderPosition);
            qreal end = QT5UKUISTYLE_H::calcRadialPos(dial, dial->maximum);
            QRectF dialRect(dial->rect.topLeft(), QSizeF(handle_r, handle_r));
            dialRect.moveCenter(dial->rect.center() + QPointF(Circle_r * qCos(dp),-Circle_r * qSin(dp)));
            QRectF GrooveRect(dial->rect.topLeft(), QSizeF(Circle_r*2, Circle_r*2));
            GrooveRect.moveCenter(dial->rect.center());

            painter->setPen(QPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Base),
                                 handle_r/4, Qt::SolidLine, Qt::RoundCap));
            painter->setBrush(Qt::NoBrush);
            painter->drawArc(GrooveRect, fist * 16 * 180 / M_PI, (end - fist) * 16 * 180 / M_PI);
            painter->setPen(QPen(highlight,handle_r/4, Qt::SolidLine, Qt::RoundCap));
            painter->drawArc(GrooveRect, fist * 16 * 180 / M_PI, (dp - fist) * 16 * 180 / M_PI);

            painter->setPen(highlight);
            painter->setBrush(highlight);
            painter->drawEllipse(dialRect);

            if(dial->subControls & SC_DialTickmarks)
            {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::WindowText));
                painter->setBrush(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::WindowText));
                painter->drawLines(QT5UKUISTYLE_H::calcLines(dial, tickOffset*2));
                painter->restore();
            }
            painter->restore();
            return;
        }
        break;
    }

    case CC_ToolButton:
    {
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            proxy()->drawPrimitive(PE_PanelButtonTool, option, painter, widget);
            proxy()->drawControl(CE_ToolButtonLabel, option, painter, widget);
//            if (option->state & State_HasFocus) {
//                painter->save();
//                painter->setPen(QPen(highLight_Click(), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//                painter->setBrush(Qt::NoBrush);
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                int x_Radius = 4;
//                int y_Radius = 4;
//                painter->drawRoundedRect(option->rect.adjusted(1, 1, -1, -1), x_Radius, y_Radius);
//                painter->restore();
//            }
            return;
        }
        break;
    }

    case CC_GroupBox: //UKUI GroupBox style
    {
        painter->save();
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            // Draw frame
            QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
            //新加
            QRect groupContents = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxContents, widget);

            if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                QStyleOptionFrame frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
                proxy()->drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                // groupBox->textColor gets the incorrect palette here
                painter->setPen(QPen(option->palette.windowText(), 1));
                int alignment = int(groupBox->textAlignment);
                if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                proxy()->drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignLeft | alignment,
                                      groupBox->palette, groupBox->state & State_Enabled, groupBox->text, QPalette::NoRole);

                if (groupBox->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect.adjusted(-2, -1, 2, 1);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Base));
                painter->drawRoundedRect(groupContents,4,4);
                painter->restore();

            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }
        }
        painter->restore();
        return;
        break;

    }

    default:        return Style::drawComplexControl(control, option, painter, widget);
    }
}

void Qt5UKUIStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_ScrollBarSlider:
    {
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return Style::drawControl(element, option, painter, widget);
        }

        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool enable = bar->state & State_Enabled;
            const bool sunKen = bar->state & State_Sunken;
            const bool mouseOver = bar->state & State_MouseOver;
            const bool horizontal = (bar->orientation == Qt::Horizontal);
            QRect rect = bar->rect;

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            animator->setAnimatorDirectionForward("slider_opacity", mouseOver);
            animator->setAnimatorDirectionForward("groove_width", mouseOver);
            if (mouseOver) {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") == 0) {
                    animator->startAnimator("slider_opacity");
                }
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") == 0) {
                    animator->startAnimator("groove_width");
                }
            } else {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") > 0) {
                    animator->startAnimator("slider_opacity");
                }
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") > 0) {
                    animator->startAnimator("groove_width");
                }
            }

            if (sunKen) {
                if (animator->currentAnimatorTime("additional_opacity") == 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", sunKen);
                    animator->startAnimator("additional_opacity");
                }
            } else {
                if (animator->currentAnimatorTime("additional_opacity") > 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", sunKen);
                    animator->startAnimator("additional_opacity");
                }
            }

            QRectF drawRect;
            qreal len = animator->value("groove_width").toReal() * 4 + 4;
            qreal m_opacity = animator->value("slider_opacity").toReal();
            qreal s_opacity = animator->value("additional_opacity").toReal();

            if (horizontal) {
                drawRect.setRect(rect.x(), rect.y() + (rect.height() - len) / 2, rect.width(), len);
            } else {
                drawRect.setRect(rect.x() + (rect.width() - len) / 2, rect.y(), len, rect.height());
            }

            painter->setBrush(mixColor(bar->palette.color(QPalette::Active, QPalette::Button),
                                       bar->palette.color(QPalette::Active, QPalette::WindowText), m_opacity + s_opacity));
            if (!enable)
                painter->setBrush(bar->palette.brush(QPalette::Disabled, QPalette::Button));
            painter->drawRoundedRect(drawRect, len / 2, len / 2);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ScrollBarAddLine: {
        return;
    }

    case CE_ScrollBarSubLine: {
        return;
    }

    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
//            if (option->state & State_HasFocus || button->features & QStyleOptionButton::DefaultButton) {
//                painter->save();
//                painter->setPen(QPen(highLight_Click(), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//                painter->setBrush(Qt::NoBrush);
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                int x_Radius = 4;
//                int y_Radius = 4;
//                painter->drawRoundedRect(button->rect.adjusted(1, 1, -1, -1), x_Radius, y_Radius);
//                painter->restore();
//            }
            return;
        }
        break;
    }

    case CE_PushButtonBevel:
    {
        proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
        return;
    }

    case CE_PushButtonLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool enable = button->state & State_Enabled;
            const bool text = !button->text.isEmpty();
            const bool icon = !button->icon.isNull();

            bool isWindowButton = false;
            bool isWindowColoseButton = false;
            bool isImportant = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("isImportant").isValid())
                    isImportant = widget->property("isImportant").toBool();

                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();
            }

            QRect drawRect = button->rect;
            int spacing = 8;
            QStyleOption sub = *option;
            if (isImportant && !(button->features & QStyleOptionButton::Flat))
                sub.state = option->state | State_On;
            else if (isWindowButton || useButtonPalette)
                sub.state = enable ? State_Enabled : State_None;
            else
                sub.state = option->state;

            if (button->features & QStyleOptionButton::HasMenu) {
                QRect arrowRect;
                int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                arrowRect.setRect(drawRect.right() - indicator, drawRect.top() + (drawRect.height() - indicator) / 2, indicator, indicator);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                if (!text && !icon)
                    spacing = 0;
                drawRect.setWidth(drawRect.width() - indicator - spacing);
                drawRect = visualRect(button->direction, button->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            }

            int tf = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                tf |= Qt::TextShowMnemonic;
            QPixmap pixmap;
            if (icon) {
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (button->state & State_On)
                    state = QIcon::On;
                pixmap = button->icon.pixmap(button->iconSize, mode, state);
            }

            QFontMetrics fm = button->fontMetrics;
            int textWidth = fm.boundingRect(option->rect, tf, button->text).width() + 2;
            int iconWidth = icon ? button->iconSize.width() : 0;
            QRect iconRect, textRect;
            if (icon && text) {
                int width = textWidth + spacing + iconWidth;
                if (width > drawRect.width()) {
                    width = drawRect.width();
                    textWidth = width - spacing - iconWidth;
                }
                textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                textRect.moveCenter(drawRect.center());
                iconRect.setRect(textRect.left(), textRect.top(), iconWidth, textRect.height());
                textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textWidth, textRect.height());
                iconRect = visualRect(option->direction, drawRect, iconRect);
                textRect = visualRect(option->direction, drawRect, textRect);
            } else if (icon) {
                iconRect = drawRect;
            } else if (text) {
                textRect = drawRect;
            }

            if (iconRect.isValid()) {
                if (widget && !widget->property("useIconHighlightEffect").isValid()) {
                    if (isImportant || isWindowColoseButton) {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    } else if (useButtonPalette || isWindowButton) {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    } else {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }

            if (textRect.isValid()) {
                if (enable) {
                    if (isWindowButton || useButtonPalette) {
                        proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                    } else {
                        if (isImportant) {
                            if (button->features & QStyleOptionButton::Flat) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                            } else {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            }
                            if (button->state & (State_MouseOver | State_Sunken | State_On)) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            }
                        } else {
                            if (button->state & (State_MouseOver | State_Sunken | State_On)) {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
                            } else {
                                proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
                            }
                        }
                    }
                } else {
                    proxy()->drawItemText(painter, textRect, tf, button->palette, false, button->text, QPalette::ButtonText);
                }
            }
            return;
        }
        break;
    }

    case CE_ToolButtonLabel:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            bool isWindowColoseButton = false;
            bool isWindowButton = false;
            bool useButtonPalette = false;
            if (widget) {
                if (widget->property("isWindowButton").isValid()) {
                    if (widget->property("isWindowButton").toInt() == 0x01)
                        isWindowButton = true;
                    if (widget->property("isWindowButton").toInt() == 0x02)
                        isWindowColoseButton = true;
                }
                if (widget->property("useButtonPalette").isValid())
                    useButtonPalette = widget->property("useButtonPalette").toBool();
            }

            const bool text = !tb->text.isEmpty();
            const bool icon = !tb->icon.isNull();
            const bool arrow = tb->features & QStyleOptionToolButton::MenuButtonPopup;
            const bool ha = tb->features & QStyleOptionToolButton::Arrow;
            const bool enable = tb->state & State_Enabled;
            QFontMetrics fm = tb->fontMetrics;
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);

            int iconWidth = (icon || ha) ? tb->iconSize.width() : 0;
            int spacing = 8;
            QRect textRect, iconRect, arrowRect;
            QRect drawRect = tb->rect;

            QStyleOption sub = *option;
            sub.state = (isWindowButton || useButtonPalette) ? (enable ? State_Enabled : State_None) : (option->state);
            if (arrow) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                drawRect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
                arrowRect.setRect(drawRect.right() - mbi + 1, drawRect.y(), mbi, drawRect.height());
                drawRect.adjust(0, 0, - (mbi + spacing), 0);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                drawRect = visualRect(option->direction, option->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            } else {
                if (tb->toolButtonStyle == Qt::ToolButtonTextOnly) {
                    drawRect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
                } else if (tb->toolButtonStyle == Qt::ToolButtonIconOnly){
                    drawRect.adjust(0, Margin_Height, 0, -Margin_Height);
                } else {
                    drawRect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
                }

            }

            int alignment = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            int textDis = fm.boundingRect(option->rect, alignment, tb->text).width() + 2;
            QPixmap pixmap;
            if (icon) {
                QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
                QIcon::Mode mode;
                if (!enable)
                    mode = QIcon::Disabled;
                else if ((tb->state & State_MouseOver) && (tb->state & State_AutoRaise))
                    mode = QIcon::Active;
                else
                    mode = QIcon::Normal;
                pixmap = tb->icon.pixmap(tb->iconSize, mode, state);
            }

            if (tb->toolButtonStyle == Qt::ToolButtonTextOnly && text) {
                textRect = drawRect;
            } else if (tb->toolButtonStyle == Qt::ToolButtonIconOnly) {
                if (icon || ha) {
                    iconRect = drawRect;
                } else if (text) {
                    textRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                if (text) {
                    int width = iconWidth + spacing + textDis;
                    if (width > drawRect.width()) {
                        width = drawRect.width();
                        textDis = drawRect.width() - iconWidth - spacing;
                    }
                    textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), iconWidth, textRect.height());
                    textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textDis, textRect.height());
                    iconRect = visualRect(option->direction, drawRect, iconRect);
                    textRect = visualRect(option->direction, drawRect, textRect);
                } else {
                    iconRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                if (text) {
                    textDis = qMax(iconWidth, textDis);
                    int height = fm.size(Qt::TextShowMnemonic, tb->text).height() + spacing + iconWidth;
                    if (textDis > drawRect.width())
                        textDis = drawRect.width();
                    textRect.setRect(drawRect.x(), drawRect.y(), textDis, height);
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), textDis, iconWidth);
                    textRect.setRect(textRect.x(), iconRect.bottom() + spacing + 1, textDis, fm.size(Qt::TextShowMnemonic, tb->text).height());
                } else {
                    iconRect = drawRect;
                }
            }

            if (textRect.isValid()) {
                proxy()->drawItemText(painter, textRect, alignment, tb->palette, enable, tb->text,
                                      (isWindowButton || useButtonPalette) ? QPalette::ButtonText
                                      : (option->state & (State_On | State_Sunken | State_MouseOver) && (option->state & State_Enabled))
                                      ? QPalette::HighlightedText : QPalette::ButtonText);
            }
            if (iconRect.isValid()) {
                sub.rect = iconRect;
                switch (tb->arrowType) {
                case Qt::UpArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowUp, &sub, painter, widget);
                    return;
                }
                case Qt::DownArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
                    return;
                }
                case Qt::LeftArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowLeft, &sub, painter, widget);
                    return;
                }
                case Qt::RightArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowRight, &sub, painter, widget);
                    return;
                }
                default:
                    break;
                }

                if (widget && !widget->property("useIconHighlightEffect").isValid()) {
                    if (isWindowColoseButton) {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    } else if (useButtonPalette || isWindowButton) {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    } else {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }
            return;
        }
        break;
    }

    case CE_TabBarTab:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            proxy()->drawControl(CE_TabBarTabShape, tab, painter, widget);
            proxy()->drawControl(CE_TabBarTabLabel, tab, painter, widget);
            return;
        }
        break;
    }

    case CE_TabBarTabShape:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            bool selected = tab->state & State_Selected;
            bool hover = tab->state & State_MouseOver;
            QRect drawRect = option->rect;
            QTransform rotMatrix;
            bool flip = false;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
            {
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -tab->rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedWest:
            {
                flip = true;
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedEast:
            {
                flip = true;
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - tab->rect.width() + 1);
                painter->setTransform(rotMatrix);
                break;
            }
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                drawRect.adjust(0, 0, 1, 0);
                QRect tmp = drawRect;
                drawRect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
            } else {
                drawRect.adjust(0, 0, 0, 1);
            }

            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth));
            bool fisttab = ((!rtlHorTabs && tab->position == QStyleOptionTab::Beginning)
                            || (rtlHorTabs && tab->position == QStyleOptionTab::End));
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs && tab->position == QStyleOptionTab::Beginning));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;

            int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, option, widget);
            if (selected || hover) {
                if (selected) {
                    if (fisttab || onlyOne) {
                        drawRect.adjust(0, 0, tabOverlap, 0);
                    } else if (lastTab) {
                        drawRect.adjust(-tabOverlap, 0, 0, 0);
                    } else {
                        drawRect.adjust(-tabOverlap, 0, tabOverlap, 0);
                    }
                }

                int TabBarTab_Radius = 6;
                QPainterPath path;
                path.moveTo(drawRect.left() + TabBarTab_Radius, drawRect.top());
                path.arcTo(QRect(drawRect.left(), drawRect.top(), TabBarTab_Radius * 2, TabBarTab_Radius * 2), 90, 90);
                path.lineTo(drawRect.left(), drawRect.bottom() - TabBarTab_Radius);
                path.arcTo(QRect(drawRect.left() - TabBarTab_Radius * 2, drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, -90);
                path.lineTo(drawRect.right() + TabBarTab_Radius, drawRect.bottom());
                path.arcTo(QRect(drawRect.right(), drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 270, -90);
                path.lineTo(drawRect.right(), drawRect.top() + TabBarTab_Radius);
                path.arcTo(QRect(drawRect.right() - TabBarTab_Radius * 2, drawRect.top(),
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, 90);
                path.lineTo(drawRect.left() + TabBarTab_Radius, drawRect.top());

                painter->setBrush(tab->palette.brush(QPalette::Active, QPalette::Base));
                if (hover && !selected)
                    painter->setBrush(mixColor(tab->palette.color(QPalette::Active, QPalette::Base),
                                               tab->palette.color(QPalette::Active, QPalette::Window), 0.6));
                painter->drawPath(path);
            } else {
                painter->setBrush(tab->palette.brush(QPalette::Active, QPalette::Window));
                painter->drawRect(drawRect);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_TabBarTabLabel:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int iconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
            bool verticalTabs = tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::RoundedWest
                    || tab->shape == QTabBar::TriangularEast || tab->shape == QTabBar::TriangularWest;
            uint alignment = Qt::AlignLeft | Qt::AlignVCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            else
                alignment |= Qt::TextHideMnemonic;

            QRect drawRect = tab->rect;
            QRect iconRect;
            QRect textRect;
            tabLayout(tab, widget, proxy(), &textRect, &iconRect);
            textRect = proxy()->subElementRect(SE_TabBarTabText, option, widget);

            painter->save();
            if (verticalTabs) {
                int newX, newY, newRot;
                if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    newX = drawRect.width() + drawRect.x();
                    newY = drawRect.y();
                    newRot = 90;
                } else {
                    newX = drawRect.x();
                    newY = drawRect.y() + drawRect.height();
                    newRot = -90;
                }
                QTransform m;
                m.translate(newX, newY);
                m.rotate(newRot);
                painter->setTransform(m, true);
            }

            if (!tab->icon.isNull()) {
                QPixmap pixmap = tab->icon.pixmap(widget ? widget->window()->windowHandle() : 0, tab->iconSize,
                                                  (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                                  (tab->state & State_Selected) ? QIcon::On : QIcon::Off);
                QPixmap drawPixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                painter->drawPixmap(iconRect.x(), iconRect.y(), drawPixmap);
            }
            proxy()->drawItemText(painter, textRect, alignment, tab->palette, tab->state & State_Enabled, tab->text, QPalette::WindowText);
            if (!(tab->state & State_Selected)) {
                int dis = ((verticalTabs ? drawRect.width() : drawRect.height()) - iconSize) / 2;
                painter->save();
                painter->resetTransform();
                painter->setPen(tab->palette.color(QPalette::Active, QPalette::Midlight));
                painter->setBrush(Qt::NoBrush);
                if (verticalTabs) {
                    if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                        painter->drawLine(drawRect.x() + dis, drawRect.bottom(), drawRect.right() - dis, drawRect.bottom());
                    } else {
                        painter->drawLine(drawRect.x() + dis, drawRect.top(), drawRect.right() - dis, drawRect.top());
                    }
                } else if (tab->direction == Qt::RightToLeft) {
                    painter->drawLine(drawRect.x(), drawRect.top() + dis, drawRect.x(), drawRect.bottom() - dis);
                } else {
                    painter->drawLine(drawRect.right(), drawRect.top() + dis, drawRect.right(), drawRect.bottom() - dis);
                }
                painter->restore();
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_ComboBoxLabel:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QRect arrowRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxArrow, widget);
            QRect editRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);

            QStyleOption arrow = *option;
            arrow.state = option->state & State_Enabled ? State_Enabled : State_None;
            arrow.rect = arrowRect;
            proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrow, painter, widget);

            painter->save();
            if (!comboBox->currentIcon.isNull()) {
                QIcon::Mode mode = comboBox->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = comboBox->currentIcon.pixmap(comboBox->iconSize, mode);
                QRect iconRect;
                if (comboBox->direction == Qt::RightToLeft) {
                    iconRect.setRect(editRect.right() - comboBox->iconSize.width(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x(), editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                } else {
                    iconRect.setRect(editRect.x(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x() + iconRect.width() + 8, editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                }
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }

            if (!comboBox->currentText.isEmpty() && !comboBox->editable) {
                proxy()->drawItemText(painter, editRect, visualAlignment(option->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                      option->palette, option->state & State_Enabled, comboBox->currentText);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_RadioButtonContents, option, widget);
            proxy()->drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_CheckBoxIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_CheckBoxContents, option, widget);
            proxy()->drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            uint alignment = visualAlignment(button->direction, Qt::AlignLeft | Qt::AlignVCenter);
            const bool enable = button->state & State_Enabled;

            if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pixmap;
            QRect textRect = button->rect;
            if (!button->icon.isNull()) {
                pixmap = button->icon.pixmap(button->iconSize, enable ? QIcon::Normal : QIcon::Disabled);
                proxy()->drawItemPixmap(painter, button->rect, alignment, pixmap);
                int spacing = 8;
                if (button->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - button->iconSize.width() - spacing);
                else
                    textRect.setLeft(textRect.left() + button->iconSize.width() + spacing);
            }
            if (!button->text.isEmpty()){
                proxy()->drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                                      button->palette, button->state & State_Enabled, button->text, QPalette::WindowText);
            }
            return;
        }
        break;
    }

    case CE_SizeGrip:
    {
        /*
             * Style is not required here, as required by design
             */
        return;

    }break;


        //Drawing of single menu item of menu bar
    case CE_MenuBarItem:
    {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect.adjusted(0, 1, 0, -3);
            painter->fillRect(option->rect, option->palette.window());

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip| Qt::TextSingleLine;

            if (!proxy()->styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            QPixmap pix = mbi->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, option, widget), QIcon::Normal);
            if (!pix.isNull())
                proxy()->drawItemPixmap(painter,mbi->rect, alignment, pix);
            else
                proxy()->drawItemText(painter, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled,
                                      mbi->text, QPalette::ButtonText);

            bool act = mbi->state & State_Selected && mbi->state & State_Sunken | mbi->state & State_HasFocus;
            bool dis = !(mbi->state & State_Enabled);


            QRect r = option->rect;
            //when hover、click and other state, begin to draw style
            if (act) {
                painter->setBrush(option->palette.highlight().color());
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(r.adjusted(0, 0, -1, -1),4,4);

                QPalette::ColorRole textRole = dis ? QPalette::Text : QPalette::HighlightedText;
                uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!Style::styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                proxy()->drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            } else {

            }
        }
        painter->restore();

        return;
        break;

    }



    case CE_MenuItem:
    {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            const bool enable(menuItem->state & State_Enabled);
            if (menuItem->menuItemType == QStyleOptionMenuItem::EmptyArea) {
                return;
            }

            QRect drawRect = menuItem->rect;
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                int SepMenuItem_Margin = 8;
                painter->save();
                QBrush SepBrush = menuItem->palette.brush(QPalette::Active, QPalette::Text);
                painter->setPen(QPen(SepBrush, 1, Qt::SolidLine, Qt::RoundCap));
                painter->setBrush(Qt::NoBrush);
                painter->setOpacity(0.1);
                painter->drawLine(QPointF(drawRect.left() + SepMenuItem_Margin, drawRect.center().y()),
                                  QPointF(drawRect.right() - SepMenuItem_Margin, drawRect.center().y()));
                painter->restore();
                return;
            }

            const bool hover(menuItem->state & State_MouseOver);
            const bool sunken(menuItem->state & State_Sunken);
            const bool selected(menuItem->state & State_Selected);
            const bool layout(menuItem->direction == Qt::LeftToRight);
            int MenuItem_xRadius = 2;
            int MenuItem_yRadius = 2;
            if (enable && (selected | hover | sunken)) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(Qt::NoPen);
                painter->setBrush(menuItem->palette.brush(QPalette::Active, QPalette::Highlight));
                painter->drawRoundedRect(drawRect, MenuItem_xRadius, MenuItem_yRadius);
                painter->restore();
            }
            int MenuItem_Spacing = 8;
            int MenuItem_VMargin = 3;
            drawRect = drawRect.adjusted(8, MenuItem_VMargin, -8, -MenuItem_VMargin);//去除item边框

            if (menuItem->menuHasCheckableItems) {
                const bool checked = menuItem->checked;
                if (menuItem->checkType == QStyleOptionMenuItem::Exclusive) {
                    if (checked) {
//                        QStyleOptionButton radioOption;
//                        radioOption.QStyleOption::operator=(*option);
//                        radioOption.rect = drawRect;
//                        radioOption.state |= State_On;
//                        radioOption.rect = proxy()->subElementRect(SE_RadioButtonIndicator, &radioOption, widget);
//                        proxy()->drawPrimitive(PE_IndicatorRadioButton, &radioOption, painter, widget);
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("dialog-ok");
                        QIcon::Mode mode = enable ?  QIcon::Active : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                } else if (menuItem->checkType == QStyleOptionMenuItem::NonExclusive) {
                    if (checked) {
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("dialog-ok");
                        QIcon::Mode mode = enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                }
            }

            if (menuItem->maxIconWidth != 0 && menuItem->checkType == QStyleOptionMenuItem::NotCheckable) {
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                if (!menuItem->icon.isNull()) {
                    QSize iconSize(smallIconSize, smallIconSize);
                    QIcon::Mode mode =  enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                    QIcon::State state = menuItem->checked ? QIcon::On : QIcon::Off;
                    QPixmap pixmap = menuItem->icon.pixmap(iconSize, mode, state);
                    QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                    QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - smallIconSize)/2, smallIconSize, smallIconSize);
                    iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                    painter->save();
                    painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPixmap(iconRect, target);
                    painter->restore();
                }
            }

            if (menuItem->menuHasCheckableItems || menuItem->maxIconWidth != 0) {
                int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(iconWidth + MenuItem_Spacing, 0, 0, 0));
            } else {
                drawRect = drawRect.adjusted(4, 0, -4, -0);//去除item边框
            }

            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
                int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
                QRect indRect(drawRect.right() - indWidth, drawRect.y() + (drawRect.height() - indHight)/2, indWidth, indHight);//箭头绘制是居中的，绘制会有写误差。
                indRect = visualRect(menuItem->direction, drawRect, indRect);
                QStyleOption indOption = *option;
                if (selected)
                    indOption.state |= State_MouseOver;
                indOption.rect = indRect;
                proxy()->drawPrimitive(layout ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft, &indOption, painter, widget);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(0, 0, -(indWidth + MenuItem_Spacing), 0));
            }

            if (!menuItem->text.isEmpty()) {
                QString text = menuItem->text;
                painter->save();
                if (enable) {
                    if (selected | hover | sunken) {
                        painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::HighlightedText));
                    } else {
                        painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                    }
                } else {
                    painter->setPen(menuItem->palette.color(QPalette::Disabled, QPalette::Text));
                }
                painter->setBrush(Qt::NoBrush);

                int text_flags = Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine;
                if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    text_flags |= Qt::TextShowMnemonic;
                int textToDraw_flags = text_flags;
                int t = menuItem->text.indexOf(QLatin1Char('\t'));
                if (t >=0) {
                    const QString textToDraw = menuItem->text.mid(t + 1);
                    text = menuItem->text.left(t);
                    if (layout)
                        textToDraw_flags |= Qt::AlignRight;
                    else
                        textToDraw_flags |= Qt::AlignLeft;
                    painter->drawText(drawRect.adjusted(0, 0, 0, -2), textToDraw_flags, textToDraw);
                }

                QFont font = menuItem->font;
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
                if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                painter->setFont(font);

                if (layout)
                    text_flags |= Qt::AlignLeft;
                else
                    text_flags |= Qt::AlignRight;
                painter->drawText(drawRect.adjusted(0, 0, 0, -2), text_flags, text);
                painter->restore();
            }
            return;
        }
        break;
    }



        // Draw UKUI ToolBoxTab Style
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            proxy()->drawControl(CE_ToolBoxTabShape, tb, painter, widget);
            proxy()->drawControl(CE_ToolBoxTabLabel, tb, painter, widget);
            return;
        }
        break;

    case CE_ToolBoxTabShape:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            if(widget->isEnabled()){
                painter->setBrush(tb->palette.button().color());
                if(option->state & State_MouseOver)
                {
                    painter->setBrush(tb->palette.button().color().darker(110));
                    if(option->state & State_Sunken)
                        painter->setBrush(tb->palette.button().color().darker(110));
                }
            }
            else {
                painter->setBrush(tb->palette.color(QPalette::Disabled,QPalette::Button));
            }

            if (tb->direction != Qt::RightToLeft|tb->direction != Qt::RightToLeft) {
                painter->drawRoundedRect(option->rect,4,4);
            }
            painter->restore();
            return;
        } break;

    case CE_ToolBoxTabLabel:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            bool enabled = tb->state & State_Enabled;
            bool selected = tb->state & State_Selected;
            int iconExtent = proxy()->pixelMetric(QStyle::PM_SmallIconSize, tb, widget);
            QPixmap pm = tb->icon.pixmap(widget ? widget->window()->windowHandle() : 0, QSize(iconExtent, iconExtent),enabled ? QIcon::Normal : QIcon::Disabled);

            QRect cr = subElementRect(QStyle::SE_ToolBoxTabContents, tb, widget);
            QRect tr, ir;
            int ih = 0;
            if (pm.isNull()) {
                tr = cr;
                tr.adjust(4, 0, -8, 0);
            } else {
                int iw = pm.width() / pm.devicePixelRatio() + 4;
                ih = pm.height()/ pm.devicePixelRatio();
                ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
                tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
            }

            if (selected && proxy()->styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, tb, widget)) {
                QFont f(painter->font());
                f.setBold(true);
                painter->setFont(f);
            }

            QString txt = tb->fontMetrics.elidedText(tb->text, Qt::ElideRight, tr.width());

            if (ih)
                painter->drawPixmap(ir.left(), (tb->rect.height() - ih) / 2, pm);

            int alignment = Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, tb, widget))
                alignment |= Qt::TextHideMnemonic;

            // painter->drawText(option->rect,tb->text, QTextOption(Qt::AlignCenter));
            proxy()->drawItemText(painter, tr, alignment, tb->palette, enabled, txt, QPalette::ButtonText);

            if (!txt.isEmpty() && option->state & State_HasFocus) {
                QStyleOptionFocusRect opt;
                opt.rect = tr;
                opt.palette = tb->palette;
                opt.state = QStyle::State_None;
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, widget);
            }
            return;
        } break;

    case CE_ProgressBar:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QStyleOptionProgressBar subOption = *pb;
            subOption.rect = proxy()->subElementRect(SE_ProgressBarGroove, pb, widget);
            proxy()->drawControl(CE_ProgressBarGroove, &subOption, painter, widget);
            subOption.rect = proxy()->subElementRect(SE_ProgressBarContents, pb, widget);
            proxy()->drawControl(CE_ProgressBarContents, &subOption, painter, widget);
            if (pb->textVisible) {
                subOption.rect = proxy()->subElementRect(SE_ProgressBarLabel, pb, widget);
                proxy()->drawControl(CE_ProgressBarLabel, &subOption, painter, widget);
            }
            return;
        }
        break;
    }

    case CE_ProgressBarGroove:
    {
        const bool enable = option->state & State_Enabled;
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
        painter->drawRoundedRect(option->rect, 4, 4);
        painter->restore();
        return;
    }

    case CE_ProgressBarContents:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            const auto progress = qMax(pb->progress, pb->minimum); // workaround for bug in QProgressBar

            const bool vertical = pb->orientation == Qt::Vertical;
            const bool inverted = pb->invertedAppearance;
            const bool indeterminate = (pb->minimum == 0 && pb->maximum == 0);

            QRect rect = pb->rect;
            int maxWidth = vertical ? pb->rect.height() : pb->rect.width();
            const auto totalSteps = qMax(Q_INT64_C(1), qint64(pb->maximum) - pb->minimum);
            const auto progressSteps = qint64(progress) - pb->minimum;
            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
            int len = indeterminate ? maxWidth : progressBarWidth;

            bool reverse = (!vertical && (pb->direction == Qt::RightToLeft)) || vertical;
            if (inverted)
                reverse = !reverse;

            QColor startColor = option->palette.color(QPalette::Active, QPalette::Highlight);
            QColor endColor = highLight_Click();
            QLinearGradient linearGradient;
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            QRect progressRect;
            int diff = 0;
            if (indeterminate) {
                len = 56;
                double currentValue = 0;
                if (QVariantAnimation *animation = m_animation_helper->animation(option->styleObject)) {
                    currentValue = animation->currentValue().toDouble();
                    if (animation->currentTime() == 0) {
                        animation->setDirection(QAbstractAnimation::Forward);
                        animation->start();
                    } else if (animation->currentTime() == animation->totalDuration()) {
                        animation->setDirection(QAbstractAnimation::Backward);
                        animation->start();
                    }
                } else {
                    m_animation_helper->startAnimation(new ProgressBarAnimation(option->styleObject));
                }
                diff = currentValue * (maxWidth - len);
            } else {
                m_animation_helper->stopAnimation(option->styleObject);
            }
            if (vertical) {
                if (reverse) {
                    progressRect.setRect(rect.left(), rect.bottom() + 1 - len - diff, rect.width(), len);
                    linearGradient.setStart(progressRect.bottomLeft());
                    linearGradient.setFinalStop(progressRect.topLeft());
                } else {
                    progressRect.setRect(rect.x(), rect.top() + diff, rect.width(), len);
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.bottomLeft());
                }
            } else {
                if (reverse) {
                    progressRect.setRect(rect.right() + 1 - len - diff, rect.top(), len, rect.height());
                    linearGradient.setStart(progressRect.topRight());
                    linearGradient.setFinalStop(progressRect.topLeft());
                } else {
                    progressRect.setRect(rect.x() + diff, rect.y(), len, rect.height());
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.topRight());
                }
            }

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(linearGradient);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(progressRect, 4, 4);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ProgressBarLabel:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            if (pb->textVisible) {
                const auto progress = qMax(pb->progress, pb->minimum);
                const bool vertical = pb->orientation == Qt::Vertical;
                const bool inverted = pb->invertedAppearance;
                const bool indeterminate = (pb->minimum == 0 && pb->maximum == 0);

                int maxWidth = vertical ? pb->rect.height() : pb->rect.width();
                const auto totalSteps = qMax(Q_INT64_C(1), qint64(pb->maximum) - pb->minimum);
                const auto progressSteps = qint64(progress) - pb->minimum;
                const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
                int len = indeterminate ? maxWidth : progressBarWidth;

                bool reverse = (!vertical && (pb->direction == Qt::RightToLeft)) || vertical;
                if (inverted)
                    reverse = !reverse;

                painter->save();
                painter->setBrush(Qt::NoBrush);
                QRect rect = pb->rect;
                if (pb->orientation == Qt::Vertical) {
                    rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());
                    QTransform m;
                    m.rotate(90);
                    m.translate(0, -rect.height());
                    painter->setTransform(m, true);
                }
                QRect textRect(rect.x(), rect.y(), pb->fontMetrics.horizontalAdvance(pb->text), rect.height());
                textRect.moveCenter(rect.center());
                if (len <= textRect.left()) {
                    painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                    painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                } else if (len >= textRect.right()) {
                    painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                    painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                } else {
                    QRect leftRect(textRect.x(), textRect.y(), len - textRect.left(), textRect.height());
                    QRect rightRect(leftRect.right() + 1, textRect.y(), textRect.right() + 1 - len, textRect.height());
                    if (reverse) {
                        leftRect.setRect(textRect.left(), textRect.top(), maxWidth - len - textRect.left(), textRect.height());
                        rightRect.setRect(leftRect.right() + 1, textRect.top(), textRect.width() - leftRect.width(), textRect.height());
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                        painter->setClipRect(rightRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                        painter->setClipRect(leftRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                    } else {
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                        painter->setClipRect(rightRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                        painter->setClipRect(leftRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                    }
                }
                painter->resetTransform();
                painter->restore();
            }
            return;
        }
        break;
    }

    case CE_Header:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRegion clipRegion = painter->clipRegion();
            painter->setClipRect(option->rect);
            proxy()->drawControl(CE_HeaderSection, option, painter, widget);
            QStyleOptionHeader subopt = *header;
            subopt.rect = proxy()->subElementRect(SE_HeaderLabel, header, widget);
            if (subopt.rect.isValid())
                proxy()->drawControl(CE_HeaderLabel, &subopt, painter, widget);
            if (header->sortIndicator != QStyleOptionHeader::None) {
                subopt.rect = proxy()->subElementRect(SE_HeaderArrow, option, widget);
                proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
            }
            painter->setClipRegion(clipRegion);
            return;
        }
        break;
    }

    case CE_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            const bool enable = header->state & State_Enabled;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(header->palette.brush(enable ? QPalette::Active : QPalette::Disabled, QPalette::Base));
            painter->drawRect(header->rect);

            painter->setPen(header->palette.color(QPalette::Active, QPalette::Midlight));
            painter->setBrush(Qt::NoBrush);
            if (header->orientation == Qt::Horizontal) {
                int iconSize = proxy()->pixelMetric(PM_SmallIconSize);
                int dis = (header->rect.height() - iconSize) / 2;
                if (header->section != 0 || header->position == QStyleOptionHeader::Beginning) {
                    if (header->direction == Qt::LeftToRight) {
                        painter->drawLine(header->rect.right(), header->rect.top() + dis, header->rect.right(), header->rect.bottom() - dis);
                    } else {
                        painter->drawLine(header->rect.left(), header->rect.top() + dis, header->rect.left(), header->rect.bottom() - dis);
                    }
                }
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderLabel:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRect rect = header->rect;
            QRect textRect = header->rect;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            textRect.adjust(margin, 0, -margin, -0);

            int iconSize = proxy()->pixelMetric(PM_SmallIconSize);
            if (!header->icon.isNull()) {
                QPixmap pixmap = header->icon.pixmap(iconSize, iconSize, header->state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
                QRect iconRect(textRect.x(), textRect.y() + (textRect.height() - iconSize) / 2, iconSize, iconSize);
                textRect.setRect(iconRect.right() + 1 + 8, textRect.y(), textRect.width() - iconRect.width() - 8, textRect.height());
                iconRect = visualRect(header->direction, rect, iconRect);
                textRect = visualRect(header->direction, rect, textRect);
                painter->drawPixmap(iconRect, pixmap);
            }

            if (header->state & QStyle::State_On) {
                QFont font = painter->font();
                font.setBold(true);
                painter->setFont(font);
            }
            proxy()->drawItemText(painter, textRect, header->textAlignment | Qt::AlignVCenter, header->palette,
                                  (header->state & State_Enabled), header->text, QPalette::ButtonText);
            return;
        }
        break;
    }

    case CE_HeaderEmptyArea:
    {
        const bool enable = option->state & State_Enabled;
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(enable ? QPalette::Active : QPalette::Disabled, QPalette::Base));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case CE_ItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            painter->save();
            if (painter->clipPath().isEmpty())
                painter->setClipRect(option->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, option, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, option, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, option, widget);

            proxy()->drawPrimitive(PE_PanelItemViewItem, option, painter, widget);

            if (vi->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionButton option;
                option.rect = checkRect;
                option.state = vi->state & ~QStyle::State_HasFocus;

                switch (vi->checkState) {
                case Qt::Unchecked:
                    option.state |= QStyle::State_Off;
                    break;
                case Qt::PartiallyChecked:
                    option.state |= QStyle::State_NoChange;
                    break;
                case Qt::Checked:
                    option.state |= QStyle::State_On;
                    break;
                }
                proxy()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, painter, widget);
            }

            if (!vi->icon.isNull()) {
                QIcon::Mode mode = QIcon::Normal;
                if (!(vi->state & QStyle::State_Enabled))
                    mode = QIcon::Disabled;
                else if (vi->state & QStyle::State_Selected)
                    mode = QIcon::Selected;
                QIcon::State state = vi->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
                QPixmap pixmap = vi->icon.pixmap(vi->decorationSize, mode, state);
                QStyle::drawItemPixmap(painter, iconRect, vi->decorationAlignment, pixmap);
            }

            if (!vi->text.isEmpty()) {
                QPalette::ColorGroup cg = vi->state & QStyle::State_Enabled
                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vi->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if (((vi->decorationPosition == QStyleOptionViewItem::Top) || (vi->decorationPosition ==  QStyleOptionViewItem::Bottom))
                        && !(vi->state & State_Selected)) {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                } else if (vi->state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
                    painter->setPen(vi->palette.color(cg, QPalette::HighlightedText));
                } else {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                }

                if (vi->state & QStyle::State_Editing) {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                    painter->setBrush(QColor(Qt::red));
                    painter->drawRect(textRect.adjusted(0, 0, -1, -1));
                }
                viewItemDrawText(painter, vi, textRect);
            }
            painter->restore();
            return;
        }
        break;
    }

    default:
        return Style::drawControl(element, option, painter, widget);
    }
}

int Qt5UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent:
        return 16;

    case PM_ScrollBarSliderMin:
        return 26;

    case PM_MaximumDragDistance:
        return -1;

    case PM_MenuPanelWidth:
        return 0;
    case PM_MenuHMargin:
        return (4 + 5);
    case PM_MenuVMargin:
        return (4 + 5);

    case PM_SmallIconSize:
        return 16;

    case PM_SubMenuOverlap:return 2;

    case PM_DefaultFrameWidth:
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            return 4;
        }
        return 2;

    case PM_MenuBarItemSpacing:return 16;
    case PM_MenuBarVMargin:return 4;
    case PM_ToolTipLabelFrameWidth:return 7;

    case PM_SliderTickmarkOffset:
        return 5;
    case PM_SliderLength:
    case PM_SliderThickness:
    case PM_SliderControlThickness:
    {
        return 16;
    }
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return 8;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return 8;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarItemSpacing:
        return 4;

    case PM_MessageBoxIconSize:
        return 24;

    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        return 20;

    case PM_TabBarTabHSpace:
        return 8 * 2;
    case PM_TabBarTabVSpace:
        return 4 * 2;

    case PM_TabBarTabOverlap:
        return 1;

    case PM_TabBarScrollButtonWidth:
        return 16;

    case PM_TabBar_ScrollButtonOverlap:
        return 3;

    case PM_TabBarIconSize:
        return 16;

    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_TabBarTabShiftHorizontal:
        return 0;

    case PM_TabBarBaseHeight:
        return 2;

    case PM_TabBarBaseOverlap:
        return 0;

    case PM_ExclusiveIndicatorWidth:
        return 16;
    case PM_ExclusiveIndicatorHeight:
        return 16;
    case PM_RadioButtonLabelSpacing:
        return 8;
    case PM_IndicatorWidth:
        return 16;
    case PM_IndicatorHeight:
        return 16;

    case PM_ButtonIconSize:
        return 16;
    case PM_ButtonMargin:
        return 16;
    case PM_MenuButtonIndicator:
        return 16;
    case PM_ButtonDefaultIndicator:
        return 0;

    case PM_ComboBoxFrameWidth:
        return 2;

    case PM_SpinBoxFrameWidth:
        return 2;

    case PM_ProgressBarChunkWidth:
        return 9;

    case PM_HeaderMargin:
        return 2;
    case PM_HeaderMarkSize:
        return 16;
    case PM_ScrollView_ScrollBarOverlap:
        return 0;

    case PM_ListViewIconSize:
        return 16;
    case PM_IconViewIconSize:
        return 32;
    case PM_FocusFrameHMargin:
        return 3;

    case PM_TreeViewIndentation:
        return 20;

    default:
        break;
    }
    return Style::pixelMetric(metric, option, widget);
}

QRect Qt5UKUIStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const QRect rect = bar->rect;
            const bool horizontal = bar->orientation == Qt::Horizontal;
            int distance = 4;
            int maxlen = horizontal ? rect.width() - distance: rect.height() - distance;
            int sliderlen = 0;
            if (bar->maximum != bar->minimum) {
                uint range = bar->maximum - bar->minimum;
                sliderlen = (qint64(bar->pageStep) * maxlen) / (range + bar->pageStep);

                int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, bar, widget);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sliderPositionFromValue(bar->minimum, bar->maximum, bar->sliderPosition, maxlen - sliderlen, bar->upsideDown) + distance / 2;
            switch (subControl) {
            case SC_ScrollBarSubLine:
            case SC_ScrollBarAddLine:
            case SC_ScrollBarFirst:
            case SC_ScrollBarLast:
                return QRect();

            case SC_ScrollBarSubPage:
            {
                if (horizontal)
                    return QRect(0, 0, sliderstart, rect.height());
                else
                    return QRect(0, 0, rect.width(), sliderstart);
            }

            case SC_ScrollBarAddPage:
            {
                if (horizontal)
                    return QRect(sliderstart + sliderlen, 0, rect.width() - sliderstart - sliderlen, rect.height());
                else
                    return QRect(0, sliderstart + sliderlen, rect.width(), rect.height() - sliderstart - sliderlen);
            }


            case SC_ScrollBarSlider:
            {
                if (horizontal)
                    return QRect(sliderstart, 0, sliderlen, rect.height());
                else
                    return QRect(0, sliderstart, rect.width(), sliderlen);
            }

            case SC_ScrollBarGroove:
            {
                return rect;
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_Slider:
    {
        if(const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            const bool horizontal = slider->state & State_Horizontal;
            int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, slider, widget);
            int tickHandle = proxy()->pixelMetric(PM_SliderControlThickness, slider, widget);
            int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int sliderPos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                            (horizontal ? slider->rect.width() : slider->rect.height()) - len, slider->upsideDown);
            switch (subControl)
            {
            case SC_SliderGroove:
            {
                QRect GrooveRect = slider->rect;
                if (slider->orientation == Qt::Horizontal)
                {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        GrooveRect.adjust(0, tickOffset, 0, 0);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        GrooveRect.adjust(0, 0, 0, -tickOffset);
                }
                else
                {
                    if (slider->tickPosition & QSlider::TicksLeft)
                        GrooveRect.adjust(tickOffset, 0, 0, 0);
                    if (slider->tickPosition & QSlider::TicksRight)
                        GrooveRect.adjust(0, 0, -tickOffset, 0);
                }
                return GrooveRect;
            }
            case SC_SliderHandle:
            {
                QRect rect = slider->rect;
                rect.setHeight(tickHandle);
                rect.setWidth(tickHandle);
                QRect HandleRect = slider->rect;
                if (slider->orientation == Qt::Horizontal)
                {
                    if (slider->tickPosition & QSlider::TicksAbove)
                        HandleRect.adjust(0, tickOffset, 0, 0);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        HandleRect.adjust(0, 0, 0, -tickOffset);
                    rect.moveCenter(QPoint(sliderPos + rect.center().x(), HandleRect.center().ry()));
                }
                else
                {
                    if (slider->tickPosition & QSlider::TicksLeft)
                        HandleRect.adjust(tickOffset, 0, 0, 0);
                    if (slider->tickPosition & QSlider::TicksRight)
                        HandleRect.adjust(0, 0, -tickOffset, 0);
                    rect.moveCenter(QPoint(HandleRect.center().rx(), sliderPos + rect.center().y()));
                }
                return rect;
            }
            case SC_SliderTickmarks:
            {
                break;
            }
            default:
                break;
            }
        }
        break;
    }

    case CC_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, tb, widget);
            int MarginWidth = 10;
            QRect rect = tb->rect;
            switch (subControl) {
            case SC_ToolButton:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(0, 0, - (mbi + MarginWidth), 0);
                break;
            }
            case SC_ToolButtonMenu:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(rect.width() - (mbi + MarginWidth), 0, 0, 0);
                break;
            }
            default:
                break;
            }
            rect = visualRect(tb->direction, tb->rect, rect);
            return rect;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            QRect rect = option->rect.adjusted(comboBox_MarginWidth, comboBox_Margin, -comboBox_MarginWidth, -comboBox_Margin);

            switch (subControl) {
            case SC_ComboBoxArrow:
            {
                QRect arrowRect(rect.right() - indicator, rect.top(), indicator, rect.height());
                return visualRect(option->direction, rect, arrowRect);
            }

            case SC_ComboBoxEditField:
            {
                QRect textRect = option->rect;
                if (cb->editable) {
                    textRect.setRect(rect.left() - 2, rect.top(), rect.width() - indicator + 4, rect.height());
                } else {
                    textRect.setRect(rect.left(), rect.top(), rect.width() - indicator - 8, rect.height());
                }
                return visualRect(option->direction, option->rect, textRect);
            }

            case SC_ComboBoxListBoxPopup:
            {
                return option->rect.adjusted(0, 0, 0, 4);
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = sb->rect.height() / 2;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            const int buttonWidth = 32;
            QRect rect = option->rect;
            switch (subControl) {
            case SC_SpinBoxUp:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                rect = QRect(rect.right() + 1 - buttonWidth, rect.top(), buttonWidth, center);
                break;
            }
            case SC_SpinBoxDown:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                rect = QRect(rect.right() + 1 - buttonWidth, rect.top() + center, buttonWidth, center);
                break;
            }
            case SC_SpinBoxEditField:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons) {
                    rect = rect.adjusted(fw, 0, -fw, 0);
                } else {
                    rect = rect.adjusted(fw, 0, -(fw + buttonWidth), 0);
                }
                break;
            }
            case SC_SpinBoxFrame:
                rect = sb->rect;

            default:
                break;
            }

            return visualRect(sb->direction, sb->rect, rect);
        }
    }

    default:
        break;
    }

    return Style::subControlRect(control, option, subControl, widget);
}

QRect Qt5UKUIStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_TabBarScrollLeftButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - (buttonWidth * 2), option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect, QRect(option->rect.width() - (buttonWidth * 2), -lap,
                                                                                              buttonWidth, option->rect.height() + 2 * lap));
        return rect;
    }

    case SE_TabBarScrollRightButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());

        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - buttonWidth, option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect,
                                                       QRect(option->rect.width() - buttonWidth, -lap, buttonWidth, option->rect.height() + 2 * lap));
        return rect;
    }

    case SE_TabBarTabLeftButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->leftButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveLeft(tab->rect.left() + hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabRightButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->rightButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveRight(tab->rect.right() - hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabText:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect rect = tab->rect;
            QRect iconRect = tab->rect;
            tabLayout(tab, widget, proxy(), &rect, &iconRect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabPane:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            int overlap = proxy()->pixelMetric(PM_TabBarBaseOverlap, option, widget);
            if (twf->lineWidth == 0)
                overlap = 0;
            const QSize tabBarSize(twf->tabBarSize - QSize(overlap, overlap));
            QRect rect(twf->rect);
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect.adjust(0, tabBarSize.height(), 0, 0);
                break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect.adjust(0, 0, 0, -tabBarSize.height());
                break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                rect.adjust(0, 0, -tabBarSize.width(), 0);
                break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                rect.adjust(tabBarSize.width(), 0, 0, 0);
                break;

            default:
                break;
            }
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabContents:
    {
        if (qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            const QRect rect = proxy()->subElementRect(SE_TabWidgetTabPane, option, widget);
            int TabWidget_Margin = 4;
            return rect.adjusted(TabWidget_Margin, TabWidget_Margin, -TabWidget_Margin, -TabWidget_Margin);
        }
        break;
    }

    case SE_TabWidgetTabBar:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect rect = QRect(QPoint(0, 0), twf->tabBarSize);
            const uint alingMask = Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            {
                rect.setWidth(qMin(rect.width(), twf->rect.width() - twf->leftCornerWidgetSize.width() - twf->rightCornerWidgetSize.width()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveLeft(twf->leftCornerWidgetSize.width());
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveLeft((twf->rect.size() - twf->leftCornerWidgetSize - twf->rightCornerWidgetSize - twf->tabBarSize).width() / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveLeft(twf->rect.width() - twf->tabBarSize.width() - twf->rightCornerWidgetSize.width());
                    break;
                }
                default:
                    break;
                }
                rect = visualRect(twf->direction, twf->rect, rect);
                switch (twf->shape) {
                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), 0));
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), (twf->rect.height() - twf->tabBarSize.height()) / 2));
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), twf->rect.height() - twf->tabBarSize.height()));
                    break;
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTop(0);
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTop((twf->rect.height() - twf->tabBarSize.height()) / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                    break;
                }
                default:
                    break;
                }
                return rect;
            }
            default:
                break;
            }
        }
        break;
    }

    case SE_TabWidgetLeftCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
            QRect rect;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect = QRect(QPoint(paneRect.x(), paneRect.y() - twf->leftCornerWidgetSize.height()), twf->leftCornerWidgetSize);
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect = QRect(QPoint(paneRect.x(), paneRect.height()), twf->leftCornerWidgetSize);
                break;
            default:
                break;
            }
            rect = visualRect(twf->direction, twf->rect, rect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetRightCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
           QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
           QRect rect;
           switch (twf->shape) {
           case QTabBar::RoundedNorth:
           case QTabBar::TriangularNorth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.y() - twf->rightCornerWidgetSize.height()), twf->rightCornerWidgetSize);
               break;
           case QTabBar::RoundedSouth:
           case QTabBar::TriangularSouth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.height()), twf->rightCornerWidgetSize);
               break;
           default:
               break;
           }
           rect = visualRect(twf->direction, twf->rect, rect);
           return rect;
        }
        break;
    }

    case SE_RadioButtonIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                  proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_RadioButtonContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_CheckBoxIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                     proxy()->pixelMetric(PM_IndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_CheckBoxContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            QRect rect = option->rect;
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                rect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
            } else if (!text && icon && !(button->features & QStyleOptionButton::HasMenu)) {

            } else {
                rect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
            }
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }

    case SE_LineEditContents:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            bool clear = false;
            if (widget) {
                if (qobject_cast<QComboBox *>(widget->parent()))
                    return option->rect;
                if (widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction")))
                    clear = true;
            }

            QRect rect = f->rect;
            if (clear) {
                rect.adjust(f->lineWidth + 4, f->lineWidth, 0, -f->lineWidth);
                rect = visualRect(option->direction, option->rect, rect);
            } else {
                rect.adjust(f->lineWidth + 4, f->lineWidth, -(f->lineWidth + 4), -f->lineWidth);
            }

            return rect;
        }
        break;
    }

    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
    {
        return option->rect;
    }

    case SE_HeaderLabel:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRect rect = header->rect;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            int Header_MarginWidth = 8;
            rect.adjust(Header_MarginWidth - margin, margin, -Header_MarginWidth + margin, -margin);
            if (header->sortIndicator != QStyleOptionHeader::None && header->state & State_Horizontal) {
                int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
                rect.adjust(0, 0, -arrowSize - Header_MarginWidth, 0);
            }
            return visualRect(option->direction, header->rect, rect);
        }
        break;
    }

    case SE_HeaderArrow:
    {
        if (qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            int Header_MarginWidth = 8;
            int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
            QRect rect = option->rect.adjusted(Header_MarginWidth, margin, -Header_MarginWidth, -margin);
            QRect arrowRect(rect.right() + 1 - arrowSize, rect.y() + (rect.height() - arrowSize) / 2, arrowSize, arrowSize);
            return visualRect(option->direction, rect, arrowRect);
        }
        break;
    }

    case SE_ItemViewItemCheckIndicator:
        if (!qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            return proxy()->subElementRect(SE_CheckBoxIndicator, option, widget);
        }
        Q_FALLTHROUGH();

    case SE_ItemViewItemDecoration:
    case SE_ItemViewItemText:
    case SE_ItemViewItemFocusRect:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QRect checkRect, decorationRect, displayRect;
            viewItemLayout(vi, &checkRect, &decorationRect, &displayRect, false);
            if (element == SE_ViewItemCheckIndicator)
                return checkRect;
            else if (element == SE_ItemViewItemDecoration)
                return decorationRect;
            else if (element == SE_ItemViewItemText || element == SE_ItemViewItemFocusRect)
                return displayRect;
        }
        break;
    }

    case SE_TreeViewDisclosureItem:
        return option->rect;

    default:
        break;
    }

    return Style::subElementRect(element,option,widget);
}



// change control Qsize
QSize Qt5UKUIStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const
{
    QSize  newSize = size;
    switch (ct) {
    case CT_MenuItem: {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            int w = newSize.width();
            int MenuItem_Spacing = 8;
            if (menuItem->text.contains(QLatin1Char('\t')))
                w += 20;

            switch (menuItem->menuItemType) {
            case QStyleOptionMenuItem::SubMenu:
            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            {
                if (menuItem->menuHasCheckableItems || menuItem->maxIconWidth != 0) {
                    int iconWidth = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                    w += iconWidth + MenuItem_Spacing;
                    newSize.setHeight(qMax(iconWidth, newSize.height()));
                } else {
                    w += 8;
                }

                w += proxy()->pixelMetric(PM_IndicatorWidth, option, widget) + MenuItem_Spacing;
                newSize.setHeight(qMax(newSize.height(), proxy()->pixelMetric(PM_IndicatorHeight, option, widget)));

                int MenuItem_HMargin = 12 + 4;
                int MenuItem_VMargin = 3;
                w +=  MenuItem_HMargin;
                newSize.setWidth(qMax(w, 152));
                newSize.setHeight(qMax(newSize.height() + MenuItem_VMargin * 2, 30));
                return newSize;
            }

            case QStyleOptionMenuItem::Separator:
            {
                int SepMenuItem_HMargin = 4;
                newSize.setHeight(SepMenuItem_HMargin * 2 + 1);
                return newSize;
            }

            default:
                break;
            }
            return newSize;
        }
        break;
    }

    case CT_LineEdit:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            newSize += QSize(f->lineWidth * 2 + 8, f->lineWidth * 2);
            newSize.setWidth(qMax(newSize.width(), 140));
            newSize.setHeight(qMax(newSize.height(), 36));
            return newSize;
        }
        break;
    }

    case CT_TabBarTab:
    {
        int padding = 0;
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (!tab->icon.isNull())
                padding += 4;

            if (!tab->leftButtonSize.isEmpty() || !tab->rightButtonSize.isEmpty())
                padding += 4;

            if (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::RoundedEast
                    || tab->shape == QTabBar::TriangularWest || tab->shape == QTabBar::TriangularEast) {
                newSize.setHeight(qMax(newSize.height() + padding, 168));
                newSize.setWidth(qMax(newSize.width(), 36));
            } else {
                newSize.setWidth(qMax(newSize.width() + padding, 168));
                newSize.setHeight(qMax(newSize.height(), 36));
            }
            return newSize;
        }
        break;
    }

    case CT_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), 36));
            return newSize;
        }
        break;
    }

    case CT_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), 36));
            return newSize;
        }
        break;
    }

    case CT_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            const bool icon = !tb->icon.isNull();
            const bool text = !tb->text.isEmpty();
            int w = size.width();
            int h = size.height();
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (tb->toolButtonStyle == Qt::ToolButtonTextOnly && !(tb->features & QStyleOptionToolButton::MenuButtonPopup)) {
                w += Button_MarginWidth * 2;
            } else {
                w += ToolButton_MarginWidth * 2;
            }
            h += Margin_Height * 2;

            if (tb->toolButtonStyle != Qt::ToolButtonIconOnly) {
                QFontMetrics fm = tb->fontMetrics;
                w -= fm.horizontalAdvance(QLatin1Char(' ')) * 2;
                if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                    if (text && icon)
                        w += 4;
                    else
                        w -= 4;
                } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                    if (text && icon)
                        h += 4;
                    else
                        h -= 4;
                }
            }
            if (tb->features & QStyleOptionToolButton::MenuButtonPopup) {
                w += 8;
                newSize.setWidth(w > 60 ? w : 60);
            } else {
                newSize.setWidth(w > 36 ? w : 36);
            }
            newSize.setHeight(h > 36 ? h : 36);
            return newSize;
        }
        break;
    }

    case CT_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            int w = size.width();
            int h = size.height();
            int Margin_Height = 2;
            int ToolButton_MarginWidth = 10;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                w += Button_MarginWidth * 2;
            } else {
                w += ToolButton_MarginWidth * 2;
            }
            h += Margin_Height * 2;

            int spacing = 0;
            if (text && icon)
                spacing += 4;
            if (!text && icon)
                spacing -= 4;
            if (button->features & QStyleOptionButton::HasMenu) {
                if (icon || text)
                    spacing += 8;
            }
            w += spacing;
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget) * 2;
                w += dbw;
                h += dbw;
            }

            newSize.setWidth(w > 96 ? w : 96);
            newSize.setHeight(h > 36 ? h : 36);
            return newSize;
        }
        break;
    }

    case CT_ComboBox:
    {
        if (qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QSize newSize = size;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8 + 8 + 8 + 4;
            newSize.setWidth(qMax(newSize.width() + indicator + comboBox_MarginWidth, 160));
            newSize.setHeight(qMax(newSize.height() + comboBox_Margin, 36));
            return newSize;
        }
        break;
    }

    case CT_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            const int buttonWidth = (sb->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) != 0 ? 32 : 0;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            newSize += QSize(buttonWidth + 2 * fw, 0);
            newSize.setWidth(qMax(newSize.width(), 140));
            newSize.setHeight(qMax(newSize.height(), 36));
            return newSize;
        }
        break;
    }

    case CT_ProgressBar:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            if (pb->orientation == Qt::Vertical) {
                newSize.setWidth(newSize.width() - 8);
                newSize.setHeight(qMax(newSize.height(), 426));
            } else {
                newSize.setHeight(newSize.height() - 8);
                newSize.setWidth(qMax(newSize.width(), 426));
            }
            return newSize;
        }
        break;
    }

    case CT_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            const bool horizontal(header->orientation == Qt::Horizontal);
            const bool text(!header->text.isEmpty());
            const bool icon(!header->icon.isNull());
            int w = header->fontMetrics.size(Qt::TextShowMnemonic, header->text).width();
            int h = header->fontMetrics.size(Qt::TextShowMnemonic, header->text).height();
            int Header_MarginWidth = 8;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);

            if (icon) {
                int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                w += iconSize;
                h = qMax(iconSize, h);
                if (text)
                    w += 8;
            }
            if (horizontal && header->sortIndicator != QStyleOptionHeader::None) {
                int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
                w += arrowSize;
                h = qMax(arrowSize, h);
                if (text || icon)
                    w += 8;
            }
            h += margin * 2;
            w += Header_MarginWidth * 2;
            newSize.setWidth(w);
            newSize.setHeight(qMax(h, 36));
            return newSize;
        }
        break;
    }

    case CT_ItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QRect decorationRect, displayRect, checkRect;
            viewItemLayout(vi, &checkRect, &decorationRect, &displayRect, true);
            newSize = (decorationRect | displayRect | checkRect).size();

            int Margin_Width = 4;
            int Margin_Height = 2;
            newSize.setWidth(newSize.width() + Margin_Width * 2);
            newSize.setHeight(qMax(newSize.height() + Margin_Height * 2, 36));

            return newSize;
        }
        break;
    }

    default:
        break;
    }

    return QFusionStyle::sizeFromContents(ct, option, size, widget);;
}



void Qt5UKUIStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    qreal scale = pixmap.devicePixelRatio();
    QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size() / scale, rect);
    QRect inter = aligned.intersected(rect);

    QPixmap target = pixmap;

    auto device = painter->device();
    auto widget = dynamic_cast<QWidget *>(device);
    if (widget) {
        if (HighLightEffect::isWidgetIconUseHighlightEffect(widget)) {
            QStyleOption opt;
            opt.initFrom(widget);
            target = HighLightEffect::generatePixmap(pixmap, &opt, widget);
        }
    }

    painter->drawPixmap(inter.x(), inter.y(), target, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width() * scale, inter.height() *scale);
}



void Qt5UKUIStyle::realSetWindowSurfaceFormatAlpha(const QWidget *widget) const
{
    if (!widget)
        return;

    if (widget->testAttribute(Qt::WA_WState_Created))
        return;

    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (shouldBeTransparent(widget))
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
}

void Qt5UKUIStyle::realSetMenuTypeToMenu(const QWidget *widget) const
{
    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        if (!qobject_cast<const QMenu*>(widget)
                || widget->testAttribute(Qt::WA_X11NetWmWindowTypeMenu)
                || !widget->windowHandle())
            return;

        int wmWindowType = 0;
        if (widget->testAttribute(Qt::WA_X11NetWmWindowTypeDropDownMenu))
            wmWindowType |= QXcbWindowFunctions::DropDownMenu;
        if (widget->testAttribute(Qt::WA_X11NetWmWindowTypePopupMenu))
            wmWindowType |= QXcbWindowFunctions::PopupMenu;
        if (wmWindowType == 0) return;
        QXcbWindowFunctions::setWmWindowType(widget->windowHandle(),
                                             static_cast<QXcbWindowFunctions::WmWindowType>(wmWindowType));
    }
}



QRect  Qt5UKUIStyle::centerRect(const QRect &rect, int width, int height) const
{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }
