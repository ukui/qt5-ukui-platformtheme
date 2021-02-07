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

//---copy from qcommonstyle
#include <QTextLayout>

#include <private/qtextengine_p.h>

static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth, int maxHeight = -1, int *lastVisibleLine = nullptr)
{
    if (lastVisibleLine)
        *lastVisibleLine = -1;
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    int i = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
        // we assume that the height of the next line is the same as the current one
        if (maxHeight > 0 && lastVisibleLine && height + line.height() > maxHeight) {
            const QTextLine nextLine = textLayout.createLine();
            *lastVisibleLine = nextLine.isValid() ? i : -1;
            break;
        }
        ++i;
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}

QString calculateElidedText(const QString &text, const QTextOption &textOption,
                            const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                            Qt::TextElideMode textElideMode, int flags,
                            bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition)
{
    QTextLayout textLayout(text, font);
    textLayout.setTextOption(textOption);

    // In AlignVCenter mode when more than one line is displayed and the height only allows
    // some of the lines it makes no sense to display those. From a users perspective it makes
    // more sense to see the start of the text instead something inbetween.
    const bool vAlignmentOptimization = paintStartPosition && valign.testFlag(Qt::AlignVCenter);

    int lastVisibleLine = -1;
    viewItemTextLayout(textLayout, textRect.width(), vAlignmentOptimization ? textRect.height() : -1, &lastVisibleLine);

    const QRectF boundingRect = textLayout.boundingRect();
    // don't care about LTR/RTL here, only need the height
    const QRect layoutRect = QStyle::alignedRect(Qt::LayoutDirectionAuto, valign,
                                                 boundingRect.size().toSize(), textRect);

    if (paintStartPosition)
        *paintStartPosition = QPointF(textRect.x(), layoutRect.top());

    QString ret;
    qreal height = 0;
    const int lineCount = textLayout.lineCount();
    for (int i = 0; i < lineCount; ++i) {
        const QTextLine line = textLayout.lineAt(i);
        height += line.height();

        // above visible rect
        if (height + layoutRect.top() <= textRect.top()) {
            if (paintStartPosition)
                paintStartPosition->ry() += line.height();
            continue;
        }

        const int start = line.textStart();
        const int length = line.textLength();
        const bool drawElided = line.naturalTextWidth() > textRect.width();
        bool elideLastVisibleLine = lastVisibleLine == i;
        if (!drawElided && i + 1 < lineCount && lastVisibleLineShouldBeElided) {
            const QTextLine nextLine = textLayout.lineAt(i + 1);
            const int nextHeight = height + nextLine.height() / 2;
            // elide when less than the next half line is visible
            if (nextHeight + layoutRect.top() > textRect.height() + textRect.top())
                elideLastVisibleLine = true;
        }

        QString text = textLayout.text().mid(start, length);
        if (drawElided || elideLastVisibleLine) {
            if (elideLastVisibleLine) {
                if (text.endsWith(QChar::LineSeparator))
                    text.chop(1);
                text += QChar(0x2026);
            }
            const QStackTextEngine engine(text, font);
            ret += engine.elidedText(textElideMode, textRect.width(), flags);

            // no newline for the last line (last visible or real)
            // sometimes drawElided is true but no eliding is done so the text ends
            // with QChar::LineSeparator - don't add another one. This happened with
            // arabic text in the testcase for QTBUG-72805
            if (i < lineCount - 1 &&
                    !ret.endsWith(QChar::LineSeparator))
                ret += QChar::LineSeparator;
        } else {
            ret += text;
        }

        // below visible text, can stop
        if ((height + layoutRect.top() >= textRect.bottom()) ||
                (lastVisibleLine >= 0 && lastVisibleLine == i))
            break;
    }
    return ret;
}

QString toolButtonElideText(const QStyleOptionToolButton *option,
                            const QRect &textRect, int flags)
{
//    if (option->fontMetrics.horizontalAdvance(option->text) <= textRect.width())
    if (option->fontMetrics.width(option->text, -1) <= textRect.width())
        return option->text;

    QString text = option->text;
    text.replace('\n', QChar::LineSeparator);
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);

    return calculateElidedText(text, textOption,
                               option->font, textRect, Qt::AlignTop,
                               Qt::ElideMiddle, flags,
                               false, nullptr);
}

static QWindow *qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : 0;
}

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;

    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *toolbutton;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

void Qt5UKUIStyle::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const
{
    const QWidget *widget = option->widget;
    const int textMargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);
    textOption.setAlignment(QStyle::visualAlignment(option->direction, option->displayAlignment));

    QPointF paintPosition;
    const QString newText = calculateElidedText(option->text, textOption,
                                                option->font, textRect, option->displayAlignment,
                                                option->textElideMode, 0,
                                                true, &paintPosition);

    QTextLayout textLayout(newText, option->font);
    textLayout.setTextOption(textOption);
    viewItemTextLayout(textLayout, textRect.width());
    textLayout.draw(p, paintPosition);
}
//---copy from qcommonstyle

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
    m_shadow_helper = new ShadowHelper(this);
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
    l<<"indicator-china-weather";
    l<<"kylin-video";
//    l<<"ukui-bluetooth";
    l<<"mktip";
    return l;
}

const QStringList Qt5UKUIStyle::useDefaultPalette() const
{
    QStringList l;
    l<<"kylin-assistant";
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
    case SH_Table_GridLineColor:
        return option ? option->palette.mid().color().darker().rgb() : 0;
    case SH_ComboBox_AllowWheelScrolling:
        return int(false);
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
    //ukui-light
    QColor  window_bg(231,231,231),
            window_no_bg(233,233,233),
            base_bg(255,255,255),
            base_no_bg(248, 248, 248),
            font_bg(0,0,0),
            font_br_bg(255,255,255),
            font_di_bg(0, 0, 0, 76),
            button_bg(217,217,217),
            button_ac_bg(107,142,235),
            button_di_bg(233,233,233),
            highlight_bg(55,144,250),
            tip_bg(248,248,248),
            tip_font(22,22,22),
            alternateBase(248,248,248);

        if (!useDefaultPalette().contains(qAppName()) && (m_use_dark_palette ||  (m_is_default_style && specialList().contains(qAppName())))) {
        //ukui-dark
        window_bg.setRgb(45,46,50);
        window_no_bg.setRgb(48,46,50);
        base_bg.setRgb(31,32,34);
        base_no_bg.setRgb(28,28,30);
        font_bg.setRgb(255,255,255);
        font_bg.setAlphaF(0.9);
        font_br_bg.setRgb(255,255,255);
        font_br_bg.setAlphaF(0.9);
        font_di_bg.setRgb(255,255,255);
        font_di_bg.setAlphaF(0.3);
        button_bg.setRgb(61,61,65);
        button_ac_bg.setRgb(48,48,51);
        button_di_bg.setRgb(48,48,51);
        highlight_bg.setRgb(55,144,250);
        tip_bg.setRgb(61,61,65);
        tip_font.setRgb(232,232,232);
        alternateBase.setRgb(36,35,40);
    }

    palette.setBrush(QPalette::Window,window_bg);
    palette.setBrush(QPalette::Active,QPalette::Window,window_bg);
    palette.setBrush(QPalette::Inactive,QPalette::Window,window_no_bg);
    palette.setBrush(QPalette::Disabled,QPalette::Window,window_no_bg);
    palette.setBrush(QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Active,QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Inactive,QPalette::WindowText,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::WindowText,font_di_bg);

    palette.setBrush(QPalette::Base,base_bg);
    palette.setBrush(QPalette::Active,QPalette::Base,base_bg);
    palette.setBrush(QPalette::Inactive,QPalette::Base,base_no_bg);
    palette.setBrush(QPalette::Disabled,QPalette::Base,base_no_bg);
    palette.setBrush(QPalette::Text,font_bg);
    palette.setBrush(QPalette::Active,QPalette::Text,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::Text,font_di_bg);

    //Cursor placeholder
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setBrush(QPalette::PlaceholderText,font_di_bg);
#endif

    palette.setBrush(QPalette::ToolTipBase,tip_bg);
    palette.setBrush(QPalette::ToolTipText,tip_font);

    palette.setBrush(QPalette::Highlight,highlight_bg);
    palette.setBrush(QPalette::Active,QPalette::Highlight,highlight_bg);
    palette.setBrush(QPalette::HighlightedText,font_br_bg);

    palette.setBrush(QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Active,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Inactive,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Disabled,QPalette::BrightText,font_di_bg);

    palette.setBrush(QPalette::Button,button_bg);
    palette.setBrush(QPalette::Active,QPalette::Button,button_bg);
    palette.setBrush(QPalette::Inactive,QPalette::Button,button_bg);
    palette.setBrush(QPalette::Disabled,QPalette::Button,button_di_bg);
    palette.setBrush(QPalette::ButtonText,font_bg);
    palette.setBrush(QPalette::Inactive,QPalette::ButtonText,font_bg);
    palette.setBrush(QPalette::Disabled,QPalette::ButtonText,font_di_bg);

    palette.setBrush(QPalette::AlternateBase,alternateBase);
    palette.setBrush(QPalette::Inactive,QPalette::AlternateBase,alternateBase);
    palette.setBrush(QPalette::Disabled,QPalette::AlternateBase,button_di_bg);

    return palette;
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

    if(auto tv = qobject_cast<QTableView*>(widget))
    {
       tv->setShowGrid(false);
       tv->setAlternatingRowColors(true);
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

    if (qobject_cast<QLineEdit *>(widget)) {
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

    if (qobject_cast<QMenu*>(widget)) {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        //widget->setMask(QRegion());
        return;
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

//    if(auto tv = qobject_cast<QTableView*>(widget))
//    {
//       tv->setShowGrid(true);
//       tv->setAlternatingRowColors(false);
//    }

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



void Qt5UKUIStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    //qDebug()<<"draw PE"<<element;
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
    case PE_IndicatorBranch: {
        if (qobject_cast<const QTreeView *>(widget)) {
            bool isHover = (option->state & State_MouseOver) && (option->state & ~State_Selected);
            bool isSelected = option->state & State_Selected;
            bool enable = option->state & State_Enabled;
            QColor color = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                                 QPalette::Highlight);

            QColor color2 = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                                  QPalette::HighlightedText);

            if (isSelected || isHover) {
                if (isHover && !isSelected) {
                    int h = color.hsvHue();
                    //int s = color.hsvSaturation();
                    auto base = option->palette.base().color();
                    int v = color.value();
                    color.setHsv(h, base.lightness(), v, 64);
                }
                painter->fillRect(option->rect, color);
                auto vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
                QStyleOptionViewItem tmp = *vopt;

                // for now the style paint arrow with highlight text brush when hover
                // we but don't want to highligh the indicator when hover a view item.
                if (isSelected) {
//                    tmp.state.setFlag(QStyle::State_MouseOver);
                    tmp.state |= QStyle::State_MouseOver;
                } else {
//                    tmp.state.setFlag(QStyle::State_MouseOver, false);
                    tmp.state &= ~QStyle::State_MouseOver;
                }

                tmp.palette.setColor(tmp.palette.currentColorGroup(), QPalette::Highlight, color2);
                return Style::drawPrimitive(PE_IndicatorBranch, &tmp, painter, widget);
            }
            break;
        }
        break;
    }
    case PE_PanelItemViewItem: {
        bool isIconView = false;
        auto opt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        if (!opt)
            return;
        if (opt) {
            isIconView = (opt->decorationPosition & QStyleOptionViewItem::Top);
        }
        bool isHover = (option->state & State_MouseOver) && (option->state & ~State_Selected);
        bool isSelected = option->state & State_Selected;
        bool enable = option->state & State_Enabled;
        QColor color = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                             QPalette::Highlight);

        color.setAlpha(0);
        if (isHover && !isSelected) {
            int h = color.hsvHue();
            //int s = color.hsvSaturation();
            auto base = option->palette.base().color();
            int v = color.value();
            color.setHsv(h, base.lightness(), v, 64);
        }
        if (isSelected) {
            color.setAlpha(255);
        }

        if ((qobject_cast<const QListView *>(widget) || qobject_cast<const QListWidget *>(widget))
                && (opt->decorationPosition != QStyleOptionViewItem::Top)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(color);
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if (!isIconView)
            painter->fillRect(option->rect, color);
        else {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(color);
            painter->drawRoundedRect(option->rect, 6, 6);
            painter->restore();
        }
        return;
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

//    case PE_IndicatorHeaderArrow: //Here is the arrow drawing of the table box

//        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
//            painter->save();
//            painter->setRenderHint(QPainter::Antialiasing,true);
//            painter->setBrush(Qt::NoBrush);
//            if(option->state & State_Enabled){
//                painter->setPen(QPen(option->palette.foreground().color(), 1.1));
//                if (option->state & State_MouseOver) {
//                    painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
//                }
//            }
//            else {
//                painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
//            }
//            QPolygon points(4);
//            //Add 8 to center vertically
//            int x = option->rect.x()+8;
//            int y = option->rect.y()+8;

//            int w = 8;
//            int h =  4;
//            x += (option->rect.width() - w) / 2;
//            y += (option->rect.height() - h) / 2;
//            if (header->sortIndicator & QStyleOptionHeader::SortUp) {
//                points[0] = QPoint(x, y);
//                points[1] = QPoint(x + w / 2, y + h);
//                points[2] = QPoint(x + w / 2, y + h);
//                points[3] = QPoint(x + w, y);
//            } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
//                points[0] = QPoint(x, y + h);
//                points[1] = QPoint(x + w / 2, y);
//                points[2] = QPoint(x + w / 2, y);
//                points[3] = QPoint(x + w, y + h);
//            }
//            painter->drawLine(points[0],  points[1] );
//            painter->drawLine(points[2],  points[3] );
//            painter->restore();
//            return;
//        }

    case PE_IndicatorHeaderArrow:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            QStyleOption opt;
            opt.state = header->state & State_Enabled ? State_Enabled : State_None;
            opt.rect = header->rect;
            if(header->sortIndicator & QStyleOptionHeader::SortUp)
            {
                proxy()->drawPrimitive(PE_IndicatorArrowUp,&opt,painter,widget);
            }
            else if(header->sortIndicator & QStyleOptionHeader::SortDown)
            {
                proxy()->drawPrimitive(PE_IndicatorArrowDown,&opt,painter,widget);
            }
            return;
        }
        break;
    }

    case PE_PanelItemViewRow:
    {
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option))
        {
            QPalette::ColorGroup cg = (widget ? widget->isEnabled() : (vopt->state & QStyle::State_Enabled))
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                cg = QPalette::Inactive;

            if ((vopt->state & QStyle::State_Selected) &&
                    proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget))
                painter->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::Highlight));
            else if (vopt->features & QStyleOptionViewItem::Alternate)
                painter->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::AlternateBase));

            if(qobject_cast<const QTableView*>(widget) || qobject_cast<const QTableWidget*>(widget))
            {
                const int gridHint = proxy()->styleHint(QStyle::SH_Table_GridLineColor, option, widget);
                const QColor gridColor = static_cast<QRgb>(gridHint);
                painter->save();
                painter->setPen(gridColor);
                painter->setBrush(Qt::NoBrush);
//                painter->drawLine(vopt->rect.topRight(),vopt->rect.bottomRight());
                painter->restore();
            }
           return;
        }
        break;
    }


    case PE_PanelButtonCommand://UKUI PushButton style
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto animator = m_button_animation_helper->animator(widget);

            bool isWindowButton = false;
            bool isWindowColoseButton = false;
            if (widget && widget->property("isWindowButton").isValid())
                if (widget->property("isWindowButton").toInt() == 0x01)
                    isWindowButton = true;
                else if (widget->property("isWindowButton").toInt() == 0x02)
                    isWindowColoseButton = true;

            if (!(button->state & State_Enabled)) {
                if (animator) {
                    animator->stopAnimator("SunKen");
                    animator->stopAnimator("MouseOver");
                }
                if (button->features & QStyleOptionButton::Flat)
                    return;
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect, 4, 4);
                painter->restore();
                return;
            }

            if (!(button->state & State_AutoRaise) && !(button->features & QStyleOptionButton::Flat)) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Button));
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawRoundedRect(option->rect,4,4);
                painter->restore();
            }

            if (animator == nullptr) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                if (button->state & (State_Sunken | State_On)) {
                    if (isWindowButton) {
                        QColor color = button->palette.color(QPalette::Text);
                        if (useDefaultPalette().contains(qAppName()))
                            color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.15);
                        painter->setBrush(color);
                    } else if (isWindowColoseButton) {
                        painter->setBrush(QColor("#E44C50"));
                    } else {
                        painter->setBrush(button->palette.color(QPalette::Highlight));
                    }
                } else if (button->state & State_MouseOver) {
                    if (isWindowButton) {
                        QColor color = button->palette.color(QPalette::Text);
                        if (useDefaultPalette().contains(qAppName()))
                            color = button->palette.color(QPalette::Base);
                        color.setAlphaF(0.1);
                        painter->setBrush(color);
                    } else if (isWindowColoseButton) {
                        painter->setBrush(QColor("#F86458"));
                    } else {
                        auto color = button->palette.color(QPalette::Highlight).lighter(125);
                        painter->setBrush(color);
                    }
                }
                painter->drawRoundedRect(button->rect, 4, 4);
                painter->restore();
                return;
            }

            if ((button->state & (State_Sunken | State_On)) || animator->isRunning("SunKen")
                    || animator->value("SunKen") == 1.0)
            {
                double opacity = animator->value("SunKen").toDouble();
                if(button->state & (State_Sunken | State_On))
                {
                    if(opacity == 0.0)
                    {
                        animator->setAnimatorDirectionForward("SunKen",true);
                        animator->startAnimator("SunKen");
                    }
                }
                else
                {
                    if(opacity == 1.0)
                    {
                        animator->setAnimatorDirectionForward("SunKen",false);
                        animator->startAnimator("SunKen");
                    }
                }

                painter->save();
                if (isWindowButton) {
                    QColor color = button->palette.color(QPalette::Text);
                    if (useDefaultPalette().contains(qAppName()))
                        color = button->palette.color(QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else {
                    auto color = button->palette.color(QPalette::Highlight).lighter(125);
                    painter->setBrush(color);
                }
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(button->rect,4,4);
                painter->restore();

                painter->save();
                painter->setPen(Qt::NoPen);
                if (isWindowButton) {
                    QColor color = button->palette.color(QPalette::Text);
                    if (useDefaultPalette().contains(qAppName()))
                        color = button->palette.color(QPalette::Base);
                    color.setAlphaF(0.15);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#E44C50"));
                } else {
                    painter->setBrush(button->palette.color(QPalette::Highlight));
                }
                painter->setOpacity(opacity);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(button->rect,4,4);
                painter->restore();
                return;
            }

            if(button->state & State_MouseOver || animator->isRunning("MouseOver")
                    || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver"))
            {
                double opacity = animator->value("MouseOver").toDouble();
                if(button->state & State_MouseOver)
                {
                    animator->setAnimatorDirectionForward("MouseOver",true);
                    if(opacity == 0.0)
                    {
                        animator->startAnimator("MouseOver");
                    }
                }
                else
                {
                    animator->setAnimatorDirectionForward("MouseOver",false);
                    if(opacity == 1.0)
                    {
                        animator->startAnimator("MouseOver");
                    }
                }

                painter->save();
                painter->setOpacity(opacity);
                if (isWindowButton) {
                    QColor color = button->palette.color(QPalette::Text);
                    if (useDefaultPalette().contains(qAppName()))
                        color = button->palette.color(QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else {
                    auto color = button->palette.color(QPalette::Highlight).lighter(125);
                    painter->setBrush(color);
                }
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(button->rect,4,4);
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

    case PE_PanelButtonTool://UKUI ToolBar  item style
    {
        /*
         * When the control does not require animation,please use the code annotated below
        */
//        AnimatorIface* animator =  new AnimatorIface();
//        auto button_animator = m_button_animation_helper->animator(widget);
//        if(!(animator = button_animator))
//        {
//            animator->setAnimatorDuration("SunKen",1);
//            animator->setAnimatorDuration("MouseOver",1);
//            animator->setAnimatorStartValue("SunKen",1);
//            animator->setAnimatorStartValue("MouseOver",1);
//        }

        auto animator = m_button_animation_helper->animator(widget);

        bool isWindowButton = false;
        bool isWindowColoseButton = false;
        if (widget && widget->property("isWindowButton").isValid())
            if (widget->property("isWindowButton").toInt() == 0x01)
                isWindowButton = true;
            else if (widget->property("isWindowButton").toInt() == 0x02)
                isWindowColoseButton = true;

        if(!(option->state & State_Enabled))
        {
            animator->stopAnimator("SunKen");
            animator->stopAnimator("MouseOver");
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Disabled,QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();
            return;
        }
        if(!(option->state & State_AutoRaise))
        {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();
        }

        if (animator == nullptr) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            if (option->state & (State_Sunken | State_On)) {
                if (isWindowButton) {
                    QColor color = option->palette.color(QPalette::Text);
                    if (useDefaultPalette().contains(qAppName()))
                        color = option->palette.color(QPalette::Base);
                    color.setAlphaF(0.15);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#E44C50"));
                } else {
                    painter->setBrush(option->palette.color(QPalette::Highlight));
                }
            } else if (option->state & State_MouseOver) {
                if (isWindowButton) {
                    QColor color = option->palette.color(QPalette::Text);
                    if (useDefaultPalette().contains(qAppName()))
                        color = option->palette.color(QPalette::Base);
                    color.setAlphaF(0.1);
                    painter->setBrush(color);
                } else if (isWindowColoseButton) {
                    painter->setBrush(QColor("#F86458"));
                } else {
                    auto color = option->palette.color(QPalette::Highlight).lighter(125);
                    painter->setBrush(color);
                }
            }
            painter->drawRoundedRect(option->rect, 4, 4);
            painter->restore();
            return;
        }

        if(option->state & (State_Sunken | State_On) || animator->isRunning("SunKen")
                || animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen"))
        {
            double opacity = animator->value("SunKen").toDouble();
            if(option->state & (State_Sunken | State_On))
            {
                if(opacity == 0.0)
                {
                    animator->setAnimatorDirectionForward("SunKen",true);
                    animator->startAnimator("SunKen");
                }
            }
            else
            {
                if(animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen"))
                {
                    animator->setAnimatorDirectionForward("SunKen",false);
                    animator->startAnimator("SunKen");
                }
            }

            painter->save();
            if (isWindowButton) {
                QColor color = option->palette.color(QPalette::Text);
                if (useDefaultPalette().contains(qAppName()))
                    color = option->palette.color(QPalette::Base);
                color.setAlphaF(0.1);
                painter->setBrush(color);
            } else if (isWindowColoseButton) {
                painter->setBrush(QColor("#F86458"));
            } else {
                auto color = option->palette.color(QPalette::Highlight).lighter(125);
                painter->setBrush(color);
            }
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();

            painter->save();
            painter->setPen(Qt::NoPen);
            if (isWindowButton) {
                QColor color = option->palette.color(QPalette::Text);
                if (useDefaultPalette().contains(qAppName()))
                    color = option->palette.color(QPalette::Base);
                color.setAlphaF(0.15);
                painter->setBrush(color);
            } else if (isWindowColoseButton) {
                painter->setBrush(QColor("#E44C50"));
            } else {
                painter->setBrush(option->palette.color(QPalette::Highlight));
            }
            painter->setOpacity(opacity);
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();
            return;
        }
        if(option->state & State_MouseOver || animator->isRunning("MouseOver")
                || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver"))
        {
            double opacity = animator->value("MouseOver").toDouble();
            if(option->state & State_MouseOver)
            {
                animator->setAnimatorDirectionForward("MouseOver",true);
                if(opacity == 0.0)
                {
                    animator->startAnimator("MouseOver");
                }
            }
            else
            {
                animator->setAnimatorDirectionForward("MouseOver",false);
                if(opacity == 1.0)
                {
                    animator->startAnimator("MouseOver");
                }
            }

            painter->save();
            painter->setOpacity(opacity);
            if (isWindowButton) {
                QColor color = option->palette.color(QPalette::Text);
                if (useDefaultPalette().contains(qAppName()))
                    color = option->palette.color(QPalette::Base);
                color.setAlphaF(0.1);
                painter->setBrush(color);
            } else if (isWindowColoseButton) {
                painter->setBrush(QColor("#F86458"));
            } else {
                auto color = option->palette.color(QPalette::Highlight).lighter(125);
                painter->setBrush(color);
            }
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();
            return;
        }
       return;
      }

        //Show this section when there are too many tabs
    case PE_IndicatorTabTear:
    {
        /*
         * To Do
         * Adjust to auto width instead of displaying this block
         */
        return;
    }
        break;

        //This is rare. It's a line under the item
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb
                = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();
            painter->setPen(option->palette.base().color());

            switch (tbb->shape) {
            case QTabBar::RoundedNorth: {
                QRegion region(tbb->rect);
                region -= tbb->selectedTabRect;
                painter->drawLine(tbb->rect.topLeft(), tbb->rect.topRight());
                //No more second line
                //  painter->setClipRegion(region);
                //  painter->setPen(option->palette.base().color());
                // painter->drawLine(tbb->rect.topLeft() + QPoint(0, 1), tbb->rect.topRight() + QPoint(0, 1));
            }
                break;
            case QTabBar::RoundedWest:
                painter->drawLine(tbb->rect.left(), tbb->rect.top(), tbb->rect.left(), tbb->rect.bottom());
                break;
            case QTabBar::RoundedSouth:
                painter->drawLine(tbb->rect.left(), tbb->rect.bottom(),
                                  tbb->rect.right(), tbb->rect.bottom());
                break;
            case QTabBar::RoundedEast:
                painter->drawLine(tbb->rect.topRight(), tbb->rect.bottomRight());
                break;
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularEast:
            case QTabBar::TriangularWest:
            case QTabBar::TriangularSouth:
                //painter->restore();
                Style::drawPrimitive(element, option, painter, widget);
                return;
            }
            painter->restore();
            return;
        }

        //This is the content box style in the table control
    case PE_FrameTabWidget:
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Base));
        //painter->drawRect(option->rect.x()+2,
        //option->rect.y(),option->rect.width()/2,option->rect.height()/2);
        //painter->drawRoundedRect(option->rect.x()+5,
        //option->rect.y(),option->rect.width()-4,option->rect.height(),5,5);
        painter->drawRoundedRect(option->rect.adjusted(+2,+0,-2,+0),5,5);
        /*if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QColor borderColor = option->palette.color(QPalette::Light);
            QRect rect = option->rect.adjusted(0, 0, -1, -1);

            // Shadow outline
            if (twf->shape != QTabBar::RoundedSouth) {
                rect.adjust(0, 0, 0, -1);
                QColor alphaShadow(Qt::Window);
                alphaShadow.setAlpha(15);
                painter->setPen(alphaShadow);
                painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                painter->setPen(borderColor);
            }*/

        // outline
        // painter->setPen( option->palette.color(QPalette::Light));
        //painter->drawRect(rect);
        // Inner frame highlight
        //painter->setPen(  QColor(244,0,77));
        //painter->drawRect(rect.adjusted(1, 1, -1, -1));
        // }
        painter->restore();
        return;
    }
        break ;


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
            painter->save();
            const bool enable(f->state & State_Enabled);
            const bool focus(f->state & State_HasFocus);
            const bool hover(f->state & State_MouseOver);
            QRect rect(f->rect);
            if (enable) {
                int LineEdit_xRadius = 4;
                int LineEdit_yRadius = 4;
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Active, QPalette::Button));
                painter->drawRoundedRect(f->rect, LineEdit_xRadius, LineEdit_yRadius);
                if (focus) {
                    painter->save();
                    QPen pen(f->palette.brush(QPalette::Active, QPalette::Highlight), 2);
                    pen.setJoinStyle(Qt::RoundJoin);
                    painter->setPen(pen);
                    painter->setBrush(Qt::NoBrush);
                    painter->translate(1, 1);
                    painter->drawRoundedRect(rect.adjusted(0, 0, -2, -2), LineEdit_xRadius, LineEdit_yRadius);
                    painter->restore();
                } else if (hover) {
                    painter->save();
                    painter->setPen(f->palette.color(QPalette::Active, QPalette::Highlight));
                    painter->setBrush(Qt::NoBrush);
                    painter->translate(0.5, 0.5);
                    painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), LineEdit_xRadius, LineEdit_yRadius);
                    painter->restore();
                }
            } else {
                painter->setPen(Qt::NoPen);
                painter->setBrush(f->palette.brush(QPalette::Disabled, QPalette::Button));
                painter->drawRoundedRect(f->rect, 4, 4);
            }
            painter->restore();
            return;
        }
        break;
    }



//    case PE_IndicatorCheckBox: { //UKUI CheckBox style
//        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
//            painter->save();
//            painter->translate(0.5, 0.5);
//            painter->setRenderHint(QPainter::Antialiasing,true);
//            QColor selectpen=option->palette.color(QPalette::Mid);
//            QColor selectbg;
//            QColor selectmark;
//            if (option->state&State_Enabled){
//                selectbg=option->palette.color(QPalette::Highlight);
//                selectmark=option->palette.color(QPalette::HighlightedText);
//            }
//            else{
//                selectbg=option->palette.button().color();
//                selectmark=option->palette.color(QPalette::Mid);
//            }

//            const int maxFactor = 120;
//            QColor tmp = option->palette.base().color();
//            tmp.setRed((tmp.red() * 85) / maxFactor + (option->palette.foreground().color().red() * (maxFactor - 85)) / maxFactor);
//            tmp.setGreen((tmp.green() * 85) / maxFactor + (option->palette.foreground().color().green() * (maxFactor - 85)) / maxFactor);
//            tmp.setBlue((tmp.blue() * 85) / maxFactor + (option->palette.foreground().color().blue() * (maxFactor - 85)) / maxFactor);

//            painter->setPen(tmp);
//            painter->setBrush(Qt::NoBrush);
//            if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange)
//                painter->setPen(option->palette.color(QPalette::Highlight));
//            painter->drawRoundedRect(option->rect,3,3);
//            painter->restore();
//            // if (option->state & State_MouseOver)
//            // painter->setBrush(option->palette.color(QPalette::Highlight));
//            if (option->state & State_NoChange){//Non optional status
//                selectpen=tmp;
//                selectbg=Qt::NoBrush;
//                selectmark=tmp;
//            }
//            else if (option->state & State_On) {
//                painter->save();
//                painter->translate(0.5, 0.5);
//                painter->setRenderHint(QPainter::Antialiasing,true);
//                painter->setPen(selectpen);
//                painter->setBrush(selectbg);
//                if(option->state & State_MouseOver){
//                    painter->setPen(option->palette.midlight().color());
//                    painter->setBrush( option->palette.highlight().color().lighter());
//                }
//                else if (option->state & State_Sunken) {
//                    painter->setBrush( option->palette.highlight().color().darker());
//                }
//                painter->drawRoundedRect(option->rect,3,3);
//                painter->restore();

//                // Draw checkmark
//                painter->save();
//                painter->setRenderHint(QPainter::Antialiasing,true);
//                painter->setPen(QPen(selectmark,1.1));
//                const qreal checkMarkPadding = 1 + option->rect.width() * 0.13; // at least one pixel padding
//                QPainterPath path;
//                const qreal rectHeight = option->rect.height(); // assuming height equals width
//                path.moveTo(checkMarkPadding + rectHeight * 0.11, rectHeight * 0.47);
//                path.lineTo(rectHeight * 0.5, rectHeight - checkMarkPadding);
//                path.lineTo(rectHeight - checkMarkPadding, checkMarkPadding);
//                painter->drawPath(path.translated(option->rect.topLeft()));
//                painter->restore();
//            }
//        }
//        return;
//    }

    case PE_IndicatorCheckBox:
    {
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option))
        {
            QRect rect = checkbox->rect;
            int width = rect.width();
            int heigth = rect.height();
            bool enable = checkbox->state & State_Enabled;
            bool MouseOver = checkbox->state & State_MouseOver;
            bool SunKen = checkbox->state & State_Sunken;
            QColor box = option->palette.color(QPalette::Dark);
            QColor light = option->palette.color(QPalette::Light);

            if(checkbox->state & State_Selected)
            {
                box = option->palette.color(QPalette::HighlightedText);
            }
            if(!enable)
            {
                light = box = option->palette.color(QPalette::Disabled,QPalette::ButtonText);
            }

            painter->save();
            painter->setClipRect(rect);
            painter->setRenderHint(QPainter::Antialiasing);

            painter->setPen(QPen(box,1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(rect,4,4);

            QColor color = option->palette.color(QPalette::Highlight);
            qreal h, s, v;
            color.getHsvF(&h, &s, &v);
            if(MouseOver)
            {
                color.setHsvF(h,s,v - 0.13);
                painter->setPen(QPen(color,1));
                painter->setBrush(color);
                if(checkbox->state & (State_On | State_NoChange))
                {
                    painter->setPen(QPen(option->palette.color(QPalette::Highlight).lighter(150),1));
                    painter->setBrush(option->palette.color(QPalette::Highlight).lighter(150));
                }
                painter->drawRoundedRect(rect,4,4);
            }

            if(SunKen)
            {
                color.setHsvF(h,s + 0.15,v - 0.2);
                painter->setPen(QPen(color,1));
                painter->setBrush(color);
                if(checkbox->state & (State_On | State_NoChange))
                {
                    painter->setPen(QPen(option->palette.color(QPalette::Highlight),1));
                    painter->setBrush(option->palette.color(QPalette::Highlight));
                }
                painter->drawRoundedRect(rect,4,4);
            }

            if(checkbox->state & (State_On | State_NoChange))
            {
                if(enable && !MouseOver && !SunKen)
                {
                    painter->setPen(QPen(option->palette.color(QPalette::Highlight).lighter(125),1));
                    painter->setBrush(option->palette.color(QPalette::Highlight).lighter(125));
                    painter->drawRoundedRect(rect,4,4);
                }
                QPainterPath path;
                if(checkbox->state & State_On)
                {
                    path.moveTo(width/4 + checkbox->rect.left(),heigth/2 + checkbox->rect.top());
                    path.lineTo(width*0.45 + checkbox->rect.left(),heigth*3/4 + checkbox->rect.top());
                    path.lineTo(width*3/4 + checkbox->rect.left(),heigth/4 + checkbox->rect.top());
                }
                else if(checkbox->state & State_NoChange)
                {
                    path.moveTo(width/5 + checkbox->rect.left(),heigth/2 + checkbox->rect.top());
                    path.lineTo(width*4/5 + checkbox->rect.left(),heigth/2 + checkbox->rect.top());
                }
                painter->setPen(QPen(light,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
                painter->setBrush(Qt::NoBrush);
                painter->drawPath(path);
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowUp:
    {
        QIcon icon = QIcon::fromTheme("pan-up-symbolic");
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
        QIcon icon = QIcon::fromTheme("pan-down-symbolic");
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
        QIcon icon = QIcon::fromTheme("pan-end-symbolic");
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
        QIcon icon = QIcon::fromTheme("pan-start-symbolic");
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
            QRect rect = radiobutton->rect;
            bool enable = radiobutton->state & State_Enabled;
            bool MouseOver = radiobutton->state & State_MouseOver;
            bool SunKen = radiobutton->state & State_Sunken;
            bool On = radiobutton->state & State_On;
            QColor box = option->palette.color(QPalette::Dark);
            QColor light = option->palette.color(QPalette::Light);
            if(!enable)
            {
                light = box = option->palette.color(QPalette::Disabled,QPalette::ButtonText);
            }

            const int min = qMin(rect.width(),rect.height());
            QPainterPath circle;
            const QPointF circleCenter = rect.center() + QPoint(1, 1);
            const qreal radius = (min + (min + 1) % 2) / 2.0 - 1;
            circle.addEllipse(circleCenter, radius, radius);
            QColor color = option->palette.color(QPalette::Highlight);
            qreal h, s, v;
            color.getHsvF(&h, &s, &v);

            if(radiobutton->state & State_Off) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);

                painter->setPen(box);
                painter->setBrush(Qt::NoBrush);
                if(SunKen)
                {
                    color.setHsvF(h,s + 0.15,v - 0.2);
                    painter->setPen(box);
                    painter->setBrush(color);
                }
                else if(MouseOver)
                {
                    color.setHsvF(h,s,v - 0.13);
                    painter->setPen(box);
                    painter->setBrush(color);
                }
                if (!enable)
                    painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Base));
                painter->drawPath(circle);
                painter->restore();
            } else if (On) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(box);
                painter->setBrush(option->palette.color(QPalette::Highlight).lighter(125));
                if (SunKen) {
                    painter->setPen(box);
                    painter->setBrush(option->palette.color(QPalette::Highlight));
                } else if (MouseOver) {
                    painter->setPen(box);
                    painter->setBrush(option->palette.color(QPalette::Highlight).lighter(150));
                }
                if (!enable)
                    painter->setBrush(option->palette.color(QPalette::Disabled, QPalette::Base));
                painter->drawPath(circle);
                painter->restore();

                circle = QPainterPath();
                const qreal On_radius = radius / 2.0;
                circle.addEllipse(circleCenter, On_radius, On_radius);

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(light);
                painter->drawPath(circle);
                painter->restore();
            }
            return;
        }
        break;
    }

    default:   break;
    }
    return Style::drawPrimitive(element, option, painter, widget);
}

void Qt5UKUIStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        const QStyleOptionSlider opt = *qstyleoption_cast<const QStyleOptionSlider*>(option);
        QStyleOption tmp = opt;
        //auto animatorObj = widget->findChild<QObject*>("ukui_scrollbar_default_interaction_animator");
        auto animator = m_scrollbar_animation_helper->animator(widget);
        bool enable = option->state.testFlag(QStyle::State_Enabled);
        bool mouse_over = option->state.testFlag(QStyle::State_MouseOver);
        bool is_horizontal = option->state.testFlag(QStyle::State_Horizontal);
        if (!animator) {
            return Style::drawComplexControl(control, option, painter, widget);
        }

        animator->setAnimatorDirectionForward("groove_width", mouse_over);
        if (enable) {
            if (mouse_over) {
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") < animator->totalAnimationDuration("groove_width")) {
                    animator->startAnimator("groove_width");
                }
            } else {
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") > 0) {
                    animator->startAnimator("groove_width");
                }
            }
        }
//        移上去不需要滚动条背景
//        painter->save();
//        painter->setPen(Qt::transparent);
//        painter->setBrush(tmp.palette.windowText());
//        double percent = animator->value("groove_width").toDouble();
//        painter->setOpacity(percent * 0.1);
//        auto grooveRect = option->rect;
//        if (is_horizontal) {
//            grooveRect.setY(grooveRect.height() * (1.0 - percent));
//        } else {
//            grooveRect.setX(grooveRect.width() * (1.0 - percent));
//        }
//        if (widget->property("drawScrollBarGroove").isValid()) {
//            if (!widget->property("drawScrollBarGroove").toBool()) {
//                painter->restore();
//                return QCommonStyle::drawComplexControl(control, option, painter, widget);
//            }
//        }

//        painter->drawRect(grooveRect);
//        painter->restore();

        return QCommonStyle::drawComplexControl(control, option, painter, widget);
    }

    case CC_ComboBox:
    {
        if(const QStyleOptionComboBox* combobox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            auto animator = m_combobox_animation_helper->animator(widget);
            if(animator == nullptr)
              return Style::drawComplexControl(CC_ComboBox,option,painter,widget);

            QRect rect = subControlRect(CC_ComboBox,option,SC_ComboBoxFrame,widget);
            QRect ArrowRect = subControlRect(CC_ComboBox,option,SC_ComboBoxArrow,widget);
            QStyleOptionButton button;
            button.state = combobox->state;
            button.rect = ArrowRect;
            QStyleOption arrow;
            arrow.state = State_Enabled;
            arrow.rect = ArrowRect;
            if(combobox->editable)
            {
                int fw = combobox->frame ? proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget) : 0;
                QRect EditRect = subControlRect(CC_ComboBox,option,SC_ComboBoxEditField,widget);
                rect = EditRect.adjusted(-fw,-fw,fw,fw);
                proxy()->drawPrimitive(PE_PanelButtonCommand,&button,painter,widget);
                arrow.state = combobox->state;
            }

            if(!(option->state & State_Enabled))
            {
              animator->stopAnimator("SunKen");
              animator->stopAnimator("MouseOver");
              painter->save();
              painter->setPen(Qt::NoPen);
              painter->setBrush(option->palette.color(QPalette::Disabled,QPalette::Button));
              painter->setRenderHint(QPainter::Antialiasing,true);
              painter->drawRoundedRect(rect,4,4);
              painter->restore();
              proxy()->drawPrimitive(PE_IndicatorArrowDown,&button,painter,widget);
              return;
            }

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(rect,4,4);
            painter->restore();
            proxy()->drawPrimitive(PE_IndicatorArrowDown,&arrow,painter,widget);

            if((combobox->state & (State_Sunken | State_On)) || animator->isRunning("SunKen")
                  || animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen"))
            {
              double opacity = animator->value("SunKen").toDouble();
              if(combobox->state & (State_Sunken | State_On))
              {
                  if(opacity == 0.0)
                  {
                      animator->setAnimatorDirectionForward("SunKen",true);
                      animator->startAnimator("SunKen");
                  }
              }
              else
              {
                  if(animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen"))
                  {
                      animator->setAnimatorDirectionForward("SunKen",false);
                      animator->startAnimator("SunKen");
                  }
              }

              painter->save();
              painter->setClipRect(rect);
              auto color = combobox->palette.color(QPalette::Highlight).lighter(125);
              painter->setBrush(Qt::NoBrush);
              painter->setPen(QPen(color,1,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
              painter->setRenderHint(QPainter::Antialiasing,true);
              painter->drawRoundedRect(rect,4,4);
              painter->restore();

              painter->save();
              painter->setClipRect(rect);
              painter->setBrush(Qt::NoBrush);
              painter->setPen(QPen(combobox->palette.color(QPalette::Highlight),1,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
              painter->setRenderHint(QPainter::Antialiasing,true);
              painter->drawRoundedRect(rect,4,4);
              painter->restore();
              return;
            }
            if(combobox->state & State_MouseOver || animator->isRunning("MouseOver")
                  || (animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")))
            {
              double opacity = animator->value("MouseOver").toDouble();
              if(combobox->state & State_MouseOver)
              {
                  animator->setAnimatorDirectionForward("MouseOver",true);
                  if(opacity == 0.0)
                  {
                      animator->startAnimator("MouseOver");
                  }
              }
              else
              {
                  animator->setAnimatorDirectionForward("MouseOver",false);
                  if(animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver"))
                  {
                      animator->startAnimator("MouseOver");
                  }
              }

              painter->save();
              painter->setClipRect(rect);
              auto color = combobox->palette.color(QPalette::Highlight).lighter(125);
              painter->setBrush(Qt::NoBrush);
              painter->setPen(QPen(color,1.0,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
              painter->setRenderHint(QPainter::Antialiasing,true);
              painter->setOpacity(opacity);
              painter->drawRoundedRect(rect,4,4);
              painter->restore();
              return;
            }
        }
        break;
    }

//    case CC_SpinBox:
//    {
//        const QStyleOptionSpinBox *pb=qstyleoption_cast<const QStyleOptionSpinBox*>(option);
//        QRectF r1=subControlRect(control,option,QStyle::SC_SpinBoxUp,widget);
//        QRectF r2=subControlRect(control,option,QStyle::SC_SpinBoxDown,widget);
//        // QRect r3=subControlRect(control,option,QStyle::SC_SpinBoxEditField,widget);

//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing,true);
//        painter->setPen(QPen(option->palette.color(QPalette::Button),1));
//        painter->setBrush(option->palette.color(QPalette::Button));
//        if (widget->isEnabled()) {
//            if(pb->state&QStyle::State_HasFocus){
//                painter->setPen(QPen(option->palette.color(QPalette::Highlight),1));
//            }

//            if(pb->state&State_MouseOver){
//                painter->setPen(option->palette.color(QPalette::Highlight));
//            }
//        }
//        painter->drawRoundedRect(option->rect,4,4);
//        painter->restore();

//        /*
//         * There's no PE_IndicatorSpinUp and PE_IndicatorSpinDown here, and it's drawn directly.
//        */
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing,true);
//        painter->setBrush(Qt::NoBrush);
//        if(option->state & State_Enabled){
//            painter->setPen(QPen(option->palette.foreground().color(), 1.1));
//            if (option->state & State_MouseOver) {
//                painter->restore();
//                painter->save();
//                painter->setRenderHint(QPainter::Antialiasing,true);
//                painter->setBrush(Qt::NoBrush);
//                painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
//            }
//        }
//        else {
//            painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
//        }
//        painter->fillRect(int(r1.x())-2, int(r1.y()), int(r1.width()), int(r1.height()+r2.height()),Qt::NoBrush);

//        int w = 8;
//        int h =  4;

//        QPolygon points(4);
//        int x = int(r1.x())+2;
//        int y = int(r1.y())+2;
//        points[0] = QPoint(x, y + h);
//        points[1] = QPoint(x + w / 2, y);
//        points[2] = QPoint(x + w / 2, y);
//        points[3] = QPoint(x + w, y + h);
//        painter->drawLine(points[0],  points[1] );
//        painter->drawLine(points[2],  points[3] );

//        int x2 = int(r2.x())+2;
//        int y2 = int(r2.y())+2;
//        points[0] = QPoint(x2, y2);
//        points[1] = QPoint(x2 + w / 2, y2 + h);
//        points[2] = QPoint(x2 + w / 2, y2 + h);
//        points[3] = QPoint(x2 + w, y2);
//        painter->drawLine(points[0],  points[1] );
//        painter->drawLine(points[2],  points[3] );
//        painter->restore();

//        return ;
//    }

    case CC_SpinBox:
    {
        if(const QStyleOptionSpinBox* spinbox = qstyleoption_cast<const QStyleOptionSpinBox*>(option))
        {
            bool enable = spinbox->state & State_Enabled;
            painter->save();
            painter->setPen(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
            painter->setBrush(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
            painter->drawRoundedRect(spinbox->rect,4,4);
            painter->restore();
            if(spinbox->state & State_HasFocus)
            {
                painter->save();
                painter->setPen(spinbox->palette.color(QPalette::Highlight));
                painter->setBrush(Qt::NoBrush);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(spinbox->rect,4,4);
                painter->restore();
            }
            if(spinbox->buttonSymbols != QAbstractSpinBox::NoButtons)
            {
                QStyleOptionButton upbutton, downbutton;
                upbutton.state = spinbox->state;
                downbutton.state = spinbox->state;
                QRect uprect = proxy()->subControlRect(CC_SpinBox,spinbox,SC_SpinBoxUp,widget);
                QRect downrect = proxy()->subControlRect(CC_SpinBox,spinbox,SC_SpinBoxDown,widget);
                upbutton.rect = uprect;
                downbutton.rect = downrect;
                QStyleOption uparrow, downarrow;
                uparrow.state = State_None;
                downarrow.state = State_None;
                uparrow.rect = uprect;
                downarrow.rect = downrect;
                if(spinbox->stepEnabled & QAbstractSpinBox::StepUpEnabled)
                {
                    if(upbutton.state & State_Enabled)
                    {
                        uparrow.state |= State_Enabled;
                        if(spinbox->activeSubControls == SC_SpinBoxUp)
                        {
                            proxy()->drawPrimitive(PE_PanelButtonCommand,&upbutton,painter,widget);
                            if(spinbox->state & State_MouseOver)
                                uparrow.state |= State_MouseOver;
                            if(spinbox->state & State_Sunken)
                                uparrow.state |= State_Sunken;
                        }
                    }
                }
                else
                {
                    uparrow.state &= ~State_Enabled;
                }

                if(spinbox->stepEnabled & QAbstractSpinBox::StepDownEnabled)
                {
                    if(downbutton.state & State_Enabled)
                    {
                        downarrow.state |= State_Enabled;
                        if(spinbox->activeSubControls == SC_SpinBoxDown)
                        {
                            proxy()->drawPrimitive(PE_PanelButtonCommand,&downbutton,painter,widget);
                            if(spinbox->state & State_MouseOver)
                                downarrow.state |= State_MouseOver;
                            if(spinbox->state & State_Sunken)
                                downarrow.state |= State_Sunken;
                        }
                    }
                }
                else
                {
                    downarrow.state &= ~State_Enabled;
                }
                proxy()->drawPrimitive(PE_IndicatorArrowUp,&uparrow,painter,widget);
                proxy()->drawPrimitive(PE_IndicatorArrowDown,&downarrow,painter,widget);
            }
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
        if (const QStyleOptionToolButton *toolbutton
                = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            QRect button, menuarea,rect;
            button = proxy()->subControlRect(control, toolbutton, SC_ToolButton, widget);
            menuarea = proxy()->subControlRect(control, toolbutton, SC_ToolButtonMenu, widget);
            rect = proxy()->subControlRect(CC_ToolButton,toolbutton,SC_None,widget);

            State bflags = toolbutton->state & ~State_Sunken;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver) || !(bflags & State_Enabled)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;
            if (toolbutton->state & State_Sunken) {
                //if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                mflags |= State_Sunken;
            }

            QStyleOption tool = *toolbutton;
            tool.state = bflags;
            if(mflags & (State_Sunken | State_MouseOver))
            {
                tool.state = mflags;
            }
            tool.rect = rect;
            proxy()->drawPrimitive(PE_PanelButtonTool, &tool, painter, widget);

            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.QStyleOption::operator=(*toolbutton);
                fr.rect.adjust(3, 3, -3, -3);
                if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
                    fr.rect.adjust(0, 0, -proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator,
                                                               toolbutton, widget), 0);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fr, painter, widget);
            }
            QStyleOptionToolButton label = *toolbutton;
            label.state = bflags;
            label.rect = button;
            proxy()->drawControl(CE_ToolButtonLabel, &label, painter, widget);

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (widget && widget->property("isWindowButton").isValid())
                    tool.state &= State_Enabled;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &tool, painter, widget);
            }
            /*
            ToolButton has Menu and popupmode is DelayedPopup.
            If you want to show the arrow, please remove the comment below
*/
            //            else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
            //                int mbi = qMin(button.width(),button.height())/5;
            //                QRect ir = toolbutton->rect;
            //                QStyleOptionToolButton newBtn = *toolbutton;
            //                newBtn.rect = QRect(ir.right()  - mbi -1, ir.y() + ir.height() - mbi -1, mbi, mbi);
            //                newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
            //                Qt5UKUIStyle::drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
            //            }
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
    case CE_ItemViewItem: {
        auto p = painter;
        auto opt = option;
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            p->save();
            if (p->clipPath().isEmpty())
                p->setClipRect(opt->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

            // draw the background
            proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, widget);

            // draw the check mark
            if (vopt->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionViewItem option(*vopt);
                option.rect = checkRect;
                option.state = option.state & ~QStyle::State_HasFocus;

                switch (vopt->checkState) {
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
                proxy()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, p, widget);
            }

            // draw the icon
            QIcon::Mode mode = QIcon::Normal;
            if (!(vopt->state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (vopt->state & QStyle::State_Selected)
                mode = QIcon::Selected;
            QIcon::State state = vopt->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            //vopt->icon.paint(p, iconRect, vopt->decorationAlignment, mode, state);
            auto pixmap = vopt->icon.pixmap(vopt->decorationSize,
                                            mode,
                                            state);

            QStyle::drawItemPixmap(painter, iconRect, vopt->decorationAlignment, HighLightEffect::generatePixmap(pixmap, vopt, widget));
//            auto target = pixmap;
//            if (widget) {
//                if (widget->property("useIconHighlightEffect").isValid()) {
//                    qDebug()<<widget->property("useIconHighlightEffect").toInt();
//                    HighLightEffect::HighLightMode needHandel = HighLightEffect::HighLightMode(widget->property("useIconHighlightEffect").toInt());
//                    if (needHandel) {
//                        HighLightEffect::EffectMode mode = HighLightEffect::HighlightOnly;
//                        if (widget->property("iconHighlightEffectMode").isValid()) {
//                            auto var = widget->property("iconHighlightEffectMode");
//                            mode = qvariant_cast<HighLightEffect::EffectMode>(var);
//                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false, mode);
//                        } else {
//                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false);
//                        }
//                    }
//                }
//            }
//            QStyle::drawItemPixmap(painter, iconRect, vopt->decorationAlignment, target);

            // draw the text
            if (!vopt->text.isEmpty()) {
                QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if (vopt->state & QStyle::State_Selected) {
                    p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                } else {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                }
                if (vopt->state & QStyle::State_Editing) {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                    p->drawRect(textRect.adjusted(0, 0, -1, -1));
                }

                viewItemDrawText(p, vopt, textRect);
            }

            // draw the focus rect
            /*
            if (vopt->state & QStyle::State_HasFocus) {
                QStyleOptionFocusRect o;
                o.QStyleOption::operator=(*vopt);
                o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, widget);
                o.state |= QStyle::State_KeyboardFocusChange;
                o.state |= QStyle::State_Item;
                QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                        ? QPalette::Normal : QPalette::Disabled;
                o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                                        ? QPalette::Highlight : QPalette::Window);
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
            }
            */

            p->restore();
        }
        break;
    }
//    case CE_ProgressBarGroove:{
//        const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
//        if (!bar)
//            return;
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        painter->setPen(Qt::NoPen);
//        painter->setBrush(option->palette.button());
//        int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
//        bool needAdjustBarWidth = adjustMarignx2 > 0;
//        bool vertical = (bar->orientation == Qt::Vertical);
//        auto progressBarGroveRect = option->rect;
//        if (!vertical) {
//            if (needAdjustBarWidth) {
//                progressBarGroveRect.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//            }
//        } else {
//            if (needAdjustBarWidth) {
//                progressBarGroveRect.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//            }
//        }
//        painter->drawRoundedRect(progressBarGroveRect, 4, 4);
//        painter->restore();
//        return;
//    }
//    case CE_ProgressBarContents:{
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
//            //Judgment status
//            painter->setPen(Qt::NoPen);
//            bool vertical = false;
//            bool inverted = false;
//            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);
//            bool complete = bar->progress == bar->maximum;

//            // Get extra style options if version 2
//            vertical = (bar->orientation == Qt::Vertical);
//            inverted = bar->invertedAppearance;

//            // If the orientation is vertical, we use a transform to rotate
//            // the progress bar 90 degrees clockwise.  This way we can use the
//            // same rendering code for both orientations.
//            int maxWidth = vertical? option->rect.height(): option->rect.width();
//            const auto progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
//            const auto totalSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
//            const auto progressSteps = qint64(progress) - bar->minimum;
//            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
//            int width = indeterminate ? maxWidth : progressBarWidth;

//            QRect progressBar;
//            painter->setPen(Qt::NoPen);


//            int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
//            bool needAdjustBarWidth = adjustMarignx2 > 0;

//            //Positioning
//            progressBar = option->rect;
//            if (!indeterminate) {
//                if (!inverted) {
//                    if (!vertical) {
//                        //progressBar.setRect(option->rect.left()-1, option->rect.top(), width-3,option->rect.height()-7);
//                        progressBar.setRight(width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//                        }
//                    } else {
//                        progressBar.setTop(maxWidth - width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//                        }
//                    }
//                } else {
//                    if (!vertical) {
//                        progressBar = option->rect;
//                        progressBar.setLeft(maxWidth - width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
//                        }
//                    } else {
//                        progressBar = option->rect;
//                        progressBar.setBottom(width);
//                        if (needAdjustBarWidth) {
//                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
//                        }
//                    }
//                }
//            }

//            //Brush color
//            if (!indeterminate && width > 0) {
//                painter->save();
//                if (!vertical) {
//                    painter->setPen(Qt::NoPen);

//                    QColor startcolor = option->palette.highlight().color();
//                    QColor endcolor = option->palette.highlight().color().darker(200);
//                    QLinearGradient linearGradient(QPoint(option->rect.bottomRight().x(), option->rect.bottomRight().y()),
//                                                   QPoint(option->rect.bottomLeft().x(), option->rect.bottomLeft().y()));
//                    linearGradient.setColorAt(1,startcolor);
//                    linearGradient.setColorAt(0,endcolor);
//                    painter->setBrush(QBrush(linearGradient));

//                    // painter->setBrush(option->palette.highlight().color());

//                    if (!complete && !indeterminate)
//                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

//                    painter->drawRoundedRect(progressBar, 4, 4);
//                } else {
//                    painter->setPen(Qt::NoPen);

//                    QColor startcolor = option->palette.highlight().color();
//                    QColor endcolor = option->palette.highlight().color().darker(200);
//                    QLinearGradient linearGradient(QPoint(option->rect.topLeft()),
//                                                   QPoint(option->rect.bottomLeft()));
//                    linearGradient.setColorAt(0,startcolor);
//                    linearGradient.setColorAt(1,endcolor);
//                    painter->setBrush(QBrush(linearGradient));

//                    if (!complete && !indeterminate)
//                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

//                    painter->drawRoundedRect(progressBar, 4, 4);
//                }
//                painter->restore();
//            } else {
//                //FIXME: implement waiting animation.
//                //painter->fillRect(option->rect, Qt::red);
//            }
//        }
//        painter->restore();
//        return;
//    }
//    case CE_ProgressBarLabel:{
//        return;
//    }

    case CE_ProgressBarGroove:
    {
        const bool enable = option->state &State_Enabled;
        QRect rect = proxy()->subElementRect(SE_ProgressBarGroove,option,widget);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
        painter->setBrush(option->palette.color(enable ? QPalette::Active : QPalette::Disabled,QPalette::Button));
        painter->drawRoundedRect(rect,4,4);
        painter->restore();
        return;
    }

    case CE_ProgressBarContents:
    {
       if(const QStyleOptionProgressBar* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option))
       {
            QRect rect = proxy()->subElementRect(SE_ProgressBarContents,option,widget);
            const bool vertical = bar->orientation == Qt::Vertical;
            const bool inverted = bar->invertedAppearance;
            qint64 minimum = qint64(bar->minimum);
            qint64 maximum = qint64(bar->maximum);
            qint64 progress = qint64(bar->progress);
            qint64 totalSteps = qMax(Q_INT64_C(1), maximum - minimum);
            const bool indeterminate = (minimum == maximum);
            const bool complete = (progress == minimum);

            bool reverse = (!vertical && (bar->direction == Qt::RightToLeft)) || vertical;
            if(inverted)
               reverse = !reverse;
            if(complete)
               return;

            QColor startColor = option->palette.color(QPalette::Highlight).lighter(125);
            QColor middleColor = option->palette.color(QPalette::Highlight);
            QColor endColor = option->palette.color(QPalette::Highlight).darker(110);
            QRect progressRect = rect;
            QLinearGradient linearGradient;
            if(!vertical)
            {
                qint64 progressBarWidth = (progress - bar->minimum) * rect.width() / totalSteps;
                int width = indeterminate ? rect.width() : progressBarWidth;
                if(!reverse)
                {
                    progressRect.setRect(rect.left(),rect.top(),width,rect.height());
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.topRight());
                }
                else
                {
                    progressRect.setRect(indeterminate ? rect.left() : rect.right() - width, rect.top(),width, rect.height());
                    linearGradient.setStart(progressRect.topRight());
                    linearGradient.setFinalStop(progressRect.topLeft());
                }
            }
            else
            {
                qint64 progressBarWidth = (progress - minimum) * rect.height() / totalSteps;
                int width = indeterminate ? rect.height() : progressBarWidth;
                if(!inverted)
                {
                    progressRect.setRect(rect.left(), indeterminate ? rect.top() : rect.bottom() - width, rect.width(), width);
                    linearGradient.setStart(progressRect.bottomLeft());
                    linearGradient.setFinalStop(progressRect.topLeft());
                }
                else
                {
                    progressRect.setRect(rect.left(),rect.top(),rect.width(),width);
                    linearGradient.setStart(progressRect.topLeft());
                    linearGradient.setFinalStop(progressRect.bottomLeft());
                }
            }

            linearGradient.setColorAt(0,startColor);
            linearGradient.setColorAt(0.6,middleColor);
            linearGradient.setColorAt(1,endColor);
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing,true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(linearGradient);
            painter->drawRoundedRect(progressRect,4,4);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ProgressBarLabel:
    {
        return;
    }

    case CE_ScrollBarSlider: {
        //auto animatorObj = widget->findChild<QObject*>("ukui_scrollbar_default_interaction_animator");
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return Style::drawControl(element, option, painter, widget);
        }

        bool enable = option->state.testFlag(QStyle::State_Enabled);
        bool mouse_over = option->state.testFlag(QStyle::State_MouseOver);
        bool is_horizontal = option->state.testFlag(QStyle::State_Horizontal);
        bool is_sunken = option->state.testFlag(QStyle::State_Sunken);

        //draw slider
        if (!enable) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(option->palette.windowText());
            painter->setOpacity(0.2);
            auto sliderRect = option->rect;
            if (is_horizontal) {
                sliderRect.translate(0, sliderRect.height() - 3);
                sliderRect.setHeight(2);
            } else {
                sliderRect.translate(sliderRect.width() - 3, 0);
                sliderRect.setWidth(2);
            }
            painter->drawRoundedRect(sliderRect, 1, 1);
            painter->restore();
        } else {
            auto sliderWidth = animator->value("groove_width").toDouble();
            if(COMMERCIAL_VERSION)
            {
                sliderWidth = animator->value("groove_width").toDouble()/2;

            }

            animator->setAnimatorDirectionForward("slider_opacity", mouse_over);
            if (mouse_over) {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") == 0) {
                    animator->startAnimator("slider_opacity");
                }
            } else {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") > 0) {
                    animator->startAnimator("slider_opacity");
                }
            }

            //sunken additional opacity
            if (is_sunken) {
                if (animator->currentAnimatorTime("additional_opacity") == 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", is_sunken);
                    animator->startAnimator("additional_opacity");
                }
            } else {
                if (animator->currentAnimatorTime("additional_opacity") > 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", is_sunken);
                    animator->startAnimator("additional_opacity");
                }
            }

            //draw slider
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(option->palette.windowText());
            double slider_opacity = animator->value("slider_opacity").toDouble();
            double additional_opacity = animator->value("additional_opacity").toDouble();
            painter->setOpacity(slider_opacity + additional_opacity);
            auto sliderRect = option->rect;
            if (is_horizontal) {
                sliderRect.setY(sliderRect.height() * (0.5 - sliderWidth/2));
            } else {
                sliderRect.setX(sliderRect.width() * (0.5 - sliderWidth/2));
            }

            int rectMin = qMin(sliderRect.width(), sliderRect.height());
            painter->drawRoundedRect(sliderRect, rectMin/2, rectMin/2);
            painter->restore();
        }
        return;
    }

    case CE_ScrollBarAddLine: {
        return;
    }

    case CE_ScrollBarSubLine: {
        return;
    }

    case CE_PushButtonBevel:
    {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QRect br = btn->rect;
            int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

            QStyleOptionButton tmpBtn = *btn;
            tmpBtn.rect = br;
            proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
        }
        break;
    }

    case CE_PushButtonLabel:
    {
        const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option);
        if (!button)
            return Style::drawControl(element, option, painter, widget);

        if (button->state & State_HasFocus) {
            QStyleOptionFocusRect opt;
            opt.initFrom(widget);
            Style::drawPrimitive(PE_FrameFocusRect, &opt, painter, widget);
        }

        QRect rect = button->rect;
        int bf = proxy()->pixelMetric(PM_DefaultFrameWidth,button,widget);//2
        int bm = proxy()->pixelMetric(PM_ButtonMargin,button,widget);//9
        QRect drawRect = rect.adjusted(bf,bf,-bf,-bf);
        if (button->features & QStyleOptionButton::HasMenu)
        {
            QStyleOptionButton arrow = *button;
            int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);//16
            if (button->direction == Qt::RightToLeft)
            {
                arrow.rect.setRect(drawRect.left(),drawRect.top(),indicatorSize,drawRect.height());
                drawRect = drawRect.adjusted(indicatorSize, 0, 0, 0);
            }
            else
            {
                drawRect = drawRect.adjusted(0, 0, -indicatorSize, 0);
                arrow.rect.setRect(drawRect.right(),drawRect.top(),indicatorSize,drawRect.height());
            }
            proxy()->drawPrimitive(PE_IndicatorArrowDown,&arrow,painter,widget);
        }

        //这是是否要绘制"&P" as P（带下划线）
        uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
        if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
            tf |= Qt::TextHideMnemonic;

        if (!button->icon.isNull())
        {
            //Center both icon and text
            QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
            if (mode == QIcon::Normal && button->state & State_HasFocus)
                mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (button->state & State_On)
                state = QIcon::On;
            QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);
            if(button->text.isEmpty())
            {
                proxy()->drawItemPixmap(painter, drawRect, Qt::AlignCenter, pixmap);
                return;
            }

            QRect iconRect;
            iconRect.setRect(drawRect.left(),drawRect.top(),button->iconSize.width(),button->iconSize.height());


            QRect textRect = button->fontMetrics.boundingRect(option->rect, int(tf), button->text);
            textRect.setRect(iconRect.right(),iconRect.top(),textRect.width(),textRect.height());

            QRect IconTextRect = iconRect.adjusted(0,0,textRect.width()+bm,0);

            IconTextRect.moveCenter(drawRect.center());

            iconRect.moveCenter(IconTextRect.adjusted(0,0,-textRect.width()-bm,0).center());
            textRect.moveCenter(IconTextRect.adjusted(iconRect.width(),0,0,0).center());

            iconRect = visualRect(button->direction, drawRect, iconRect);
            textRect = visualRect(button->direction, drawRect, textRect);

            proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            drawRect = textRect;
        }
        else
        {
            tf |= Qt::AlignHCenter;
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::ButtonText));
        if(button->state & (State_Sunken | State_MouseOver | State_On))
            painter->setPen(option->palette.color(QPalette::HighlightedText));
        if(!(button->state & State_Enabled))
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::ButtonText));
        proxy()->drawItemText(painter, drawRect, int(tf), button->palette, (button->state & State_Enabled),button->text);
        painter->restore();
        return;
    }

    case CE_ToolButtonLabel: {
        if (const QStyleOptionToolButton *toolbutton
                = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
//            if (toolbutton->state & (State_Sunken | State_On)) {
//                shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
//                shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
//            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                painter->setFont(toolbutton->font);
                if(toolbutton->state & (State_Sunken | State_MouseOver | State_On))
                    painter->setPen(option->palette.color(QPalette::HighlightedText));
                if(!(toolbutton->state & State_Enabled))
                    painter->setPen(option->palette.color(QPalette::Disabled,QPalette::ButtonText));
                proxy()->drawItemText(painter, rect, alignment, toolbutton->palette,
                                      option->state & State_Enabled, toolbutton->text);
            } else {
                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;
                if (!toolbutton->icon.isNull()) {
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((option->state & State_MouseOver) && (option->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->icon.pixmap(qt_getWindow(widget), toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                                 mode, state);

                    pmSize = pm.size() / pm.devicePixelRatio();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    painter->setFont(toolbutton->font);
                    QRect pr = rect,
                            tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(0, pr.height() - 1, 0, -1);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 4); //### 4 is currently hardcoded in QToolButton::sizeHint()
                        tr.adjust(pr.width(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            proxy()->drawItemPixmap(painter, QStyle::visualRect(option->direction, rect, pr), Qt::AlignCenter, pm);
                        } else {
                            drawArrow(proxy(), toolbutton, pr, painter, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    const QString text = toolButtonElideText(toolbutton, tr, alignment);
                    if(toolbutton->state & (State_Sunken | State_MouseOver | State_On))
                        painter->setPen(option->palette.color(QPalette::HighlightedText));
                    if(!(toolbutton->state & State_Enabled))
                        painter->setPen(option->palette.color(QPalette::Disabled,QPalette::ButtonText));
                    proxy()->drawItemText(painter, QStyle::visualRect(option->direction, rect, tr), alignment, toolbutton->palette,
                                          toolbutton->state & State_Enabled, text);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(proxy(), toolbutton, rect, painter, widget);
                    } else {
                        proxy()->drawItemPixmap(painter, rect, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        return;
    }

        //Draw TabBar and every item style
    case CE_TabBarTab:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            proxy()->drawControl(CE_TabBarTabShape, tab, painter, widget);
            proxy()->drawControl(CE_TabBarTabLabel, tab, painter, widget);
            return;
        }
        break;

    case CE_TabBarTabShape:
    {
        QRect rect = option->rect;
        int state = option->state;

        QColor outline =option->palette.window().color();
        QColor highlightedOutline =option->palette.window().color();
        QColor tabFrameColor =option->palette.window().color();

        painter->save();
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
            rect = option->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

            QRect r2(rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();

            //painter->setPen(d->innerContrastLine());
            painter->setPen( Qt::NoPen);

            QTransform rotMatrix;
            bool flip = false;
            //painter->setPen(shadow);
            painter->setPen( Qt::NoPen);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedWest:
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedEast:
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - rect.width() + 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                QRect tmp = rect;
                rect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
                int temp = x1;
                x1 = y1;
                y1 = temp;
                temp = x2;
                x2 = y2;
                y2 = temp;
            }

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);

            /*
             * The following colors are the check box background
             *  colors of the outer box tab or the small pop-up box tab
             */
            QColor tabFrameColor = tab->features & QStyleOptionTab::HasFrame ?
                        option->palette.base().color() :
                        option->palette.base().color();

            QLinearGradient fillGradient(rect.topLeft(), rect.bottomLeft());
            QLinearGradient outlineGradient(rect.topLeft(), rect.bottomLeft());
            QPen outlinePen =  Qt::NoPen;
            if (selected) {
                fillGradient.setColorAt(0, tabFrameColor.lighter(104));
                fillGradient.setColorAt(1, tabFrameColor);
                outlineGradient.setColorAt(1, outline);
                outlinePen =  Qt::NoPen;
            } else {
                fillGradient.setColorAt(0, option->palette.window().color());
                fillGradient.setColorAt(0.85,option->palette.window().color());
                fillGradient.setColorAt(1, option->palette.window().color());
            }

            //No special height handling when selected
            //QRect drawRect = rect.adjusted(0, selected ? 0 : 2, 0, 3);
            QRect drawRect = rect.adjusted(0, 0, 0, 3);
            painter->setPen( Qt::NoPen);
            painter->save();
            painter->setClipRect(rect.adjusted(+1, -1, +0, selected ? -2 : -3));
            painter->setBrush(fillGradient);
            painter->drawRoundedRect(drawRect.adjusted(+1, 0, +0, -1), 4.0, 4.0);
            painter->restore();

            if (selected) {
                painter->save();
                painter->setBrush(option->palette.window().color());
                painter->drawRoundedRect(QRect(option->rect.right()-5,option->rect.y(),20,option->rect.height()-3),6,6);
                painter->drawRect(option->rect.right()-15,option->rect.y()-3,20,32);
                if(option->rect.left()-15>0){
                    painter->drawRoundedRect(QRect(option->rect.left()-13,option->rect.y(),20,option->rect.height()-3),6,6);
                    painter->drawRect(option->rect.left()-10,option->rect.y()-1,25,10);
                }
                else{
                    painter->drawRect(option->rect.x()+2,option->rect.y()-3,20,32);
                    painter->restore();
                    painter->save();
                    painter->setBrush(option->palette.base().color());
                    painter->drawRoundedRect(QRect(option->rect.x()+2,option->rect.y()-1,20,36),6,6);
                }
                painter->restore();
                painter->save();
                painter->setBrush(option->palette.base().color());
                painter->drawRoundedRect(option->rect.adjusted(+7,-1,-6,-5),6,6);
                painter->restore();
            }
        }

        painter->restore();
        return;

    }break;

    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QRect editRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
            painter->save();
            QString text = cb->currentText;
            QFontMetrics fontMetrics = cb->fontMetrics;
            QTextLayout textLayout(text);
            QTextOption opt;
            opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            opt.setAlignment(Qt::AlignHCenter);

            textLayout.setTextOption(opt);
            textLayout.beginLayout();

            int width = editRect.width();

            //
            painter->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
                QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal
                                                             : QIcon::Disabled;
                QPixmap pixmap = cb->currentIcon.pixmap(qt_getWindow(widget), cb->iconSize, mode);
                QRect iconRect(editRect);
                iconRect.setWidth(cb->iconSize.width() + 4);

                iconRect = alignedRect(cb->direction,
                                       Qt::AlignLeft | Qt::AlignVCenter,
                                       iconRect.size(), editRect);
                if (cb->editable)
                    painter->fillRect(iconRect, option->palette.brush(QPalette::Base));
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

                if (cb->direction == Qt::RightToLeft)
                    editRect.translate(-4 - cb->iconSize.width(), 0);
                else
                    editRect.translate(cb->iconSize.width() + 4, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable) {
                if (fontMetrics.width(text) < width) {
                    proxy()->drawItemText(painter, editRect.adjusted(1, 0, -1, 0),
                                          visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                          cb->palette, cb->state & State_Enabled, cb->currentText);

                }else{
                    QString elidedLastLine = fontMetrics.elidedText(text, Qt::ElideRight, width - 15);
                    proxy()->drawItemText(painter, editRect.adjusted(1, 0, -1, 0),
                                          visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                          cb->palette, cb->state & State_Enabled, elidedLastLine);
                }
            }
            textLayout.endLayout();
            painter->restore();
            return;
        }
        break;


    case CE_RadioButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            uint alignment = visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);

            if (!proxy()->styleHint(SH_UnderlineShortcut, btn, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            QRect textRect = btn->rect;
            if (!btn->icon.isNull()) {
                pix = btn->icon.pixmap(widget ? widget->window()->windowHandle() : 0, btn->iconSize, btn->state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
                proxy()->drawItemPixmap(painter, btn->rect, alignment, pix);
                if (btn->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                else
                    textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
            }
            if (!btn->text.isEmpty()){
                proxy()->drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                                      btn->palette, btn->state & State_Enabled, btn->text, QPalette::WindowText);
            }
            return;
        }
        break;

    case CE_RadioButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect( SE_RadioButtonIndicator, btn, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton ,&subopt, painter, widget);
            subopt.rect = subElementRect( SE_RadioButtonContents, btn, widget);
            proxy()->drawControl( CE_RadioButtonLabel , &subopt, painter, widget);
//            if (btn->state & State_HasFocus) {
//                QStyleOptionFocusRect fropt;
//                fropt.QStyleOption::operator=(*btn);
//                fropt.rect = subElementRect(SE_RadioButtonFocusRect, btn, widget);
//                proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
//            }
            return;
        }
        break;

    case CE_CheckBox:
    {
           if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option))
           {

               QStyleOptionButton subopt = *btn;
               subopt.rect = subElementRect(SE_CheckBoxIndicator, btn, widget);
               proxy()->drawPrimitive(PE_IndicatorCheckBox,&subopt, painter, widget);
               subopt.rect = subElementRect(SE_CheckBoxContents, btn, widget);
               proxy()->drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
//               if (btn->state & State_HasFocus)
//               {
//                   QStyleOptionFocusRect fropt;
//                   fropt.QStyleOption::operator=(*btn);
//                   fropt.rect = subElementRect(SE_CheckBoxFocusRect, btn, widget);
//                   proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
//               }
           }
           break;
    }

        //Draw table header style
//    case CE_HeaderSection:
//    {
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->fillRect(option->rect, option->palette.alternateBase().color());
//        painter->restore();
//        return;
//    }break;

    case CE_Header:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            painter->save();
            painter->setClipRect(option->rect);
            proxy()->drawControl(CE_HeaderSection, header, painter, widget);
            QStyleOptionHeader subopt = *header;
            subopt.rect = proxy()->subElementRect(SE_HeaderLabel, header, widget);
            if (subopt.rect.isValid())
                proxy()->drawControl(CE_HeaderLabel, &subopt, painter, widget);
            if (header->sortIndicator != QStyleOptionHeader::None)
            {
                subopt.rect = proxy()->subElementRect(SE_HeaderArrow, option, widget);
                proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)){

            bool mouse = header->state &  QStyle::State_MouseOver;
            bool select = header->state & QStyle::State_Sunken;
            bool on = header->state & QStyle::State_On;
            bool roundedRight = false;
            //角落控件控制
            const bool isCorner( widget && widget->inherits( "QTableCornerButton" ) );
            //控件方向判断
            const bool reverseLayout( option->direction == Qt::LeftToRight );
            //对控件位置判断
            if(header->section == QStyleOptionHeader::Middle){
                roundedRight = true;
            }else if(header->section == QStyleOptionHeader::Beginning){
                roundedRight = true;
            }

            painter->save();
            painter->setPen(Qt::transparent);
            //tree头部表格控制
            const auto view = qobject_cast<const QHeaderView*>(widget);
            if (view) {
                auto treeView = qobject_cast<const QTreeView*>(view->parent());
                if (treeView){
                    painter->setBrush(header->palette.color(QPalette::Base));
                    painter->drawRect(header->rect);
                    if(!isCorner){
                        if(roundedRight){
                            painter->setPen(option->palette.color(QPalette::Button).darker(110));
                            if(reverseLayout){
                                if(header->orientation == Qt::Horizontal){
                                    painter->drawLine(header->rect.right(),header->rect.top()+4,header->rect.right(),header->rect.bottom()-4);
                                }
                            }else{
                                if(header->orientation == Qt::Horizontal){
                                    painter->drawLine(header->rect.left(),header->rect.top()+4,header->rect.left(),header->rect.bottom()-4);
                                }
                            }
                        }
                    }
                    painter->restore();
                    return;
                }
            }

            painter->setBrush(header->palette.color(QPalette::Button).lighter(105));
            painter->drawRect(header->rect);
            if(on){
                painter->setBrush(header->palette.color(QPalette::Button).darker(105));
            }
            if(mouse){
                painter->setBrush(header->palette.color(QPalette::Button));
            }
            if(select){
                painter->setBrush(header->palette.color(QPalette::Button).darker(105));
            }
            QPainterPath path_indicator1;
            path_indicator1.addRoundedRect(option->rect.adjusted(+3,+3,-3,-3),4,4);
            painter->drawPath(path_indicator1);


            //根据不同位置绘制外观
            if(!isCorner){
                if(roundedRight){
                    painter->setPen(option->palette.color(QPalette::Button).darker(110));
                    if(reverseLayout){
                        if(header->orientation == Qt::Horizontal){
                            painter->drawLine(header->rect.right(),header->rect.top()+4,header->rect.right(),header->rect.bottom()-4);
                        }
                    }else{
                        if(header->orientation == Qt::Horizontal){
                            painter->drawLine(header->rect.left(),header->rect.top()+4,header->rect.left(),header->rect.bottom()-4);
                        }
                    }
                }
            }

            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderEmptyArea:{
        bool enable = option->state & QStyle::State_Enabled;
        painter->fillRect(option->rect,option->palette.color(enable ? QPalette::Active : QPalette::Disabled,
                                                             QPalette::Button).lighter(105));
        const auto view = qobject_cast<const QHeaderView*>(widget);
        if (view) {
            auto treeView = qobject_cast<const QTreeView*>(view->parent());
            if (treeView){
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Base));
                painter->drawRect(option->rect);
                painter->restore();
                return;
            }
        }
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


    default:
        return Style::drawControl(element, option, painter, widget);
    }
}

int Qt5UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent: {
        if(COMMERCIAL_VERSION)
            return 14;
        else
            return 8;
    }

    case PM_ScrollBarSliderMin: {
        return 60;
    }

    case PM_ScrollView_ScrollBarOverlap: {
        return -10;
    }

    case PM_MenuPanelWidth:
        return 0;
    case PM_MenuHMargin:
        return (4 + 5);
    case PM_MenuVMargin:
        return (4 + 5);

    case PM_SmallIconSize:
        return 16;


    case PM_IndicatorWidth:{
        return 16;
    }
    case PM_IndicatorHeight:{
        return 16;
    }

    case PM_SubMenuOverlap:return 2;
    case PM_ButtonMargin:return  9;

    case PM_DefaultFrameWidth:
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            return 4;
        } else if (qobject_cast<const QLineEdit *>(widget)) {
            return 6;
        }
        return 2;

    case PM_TabBarTabVSpace:return 20;
    case PM_TabBarTabHSpace:return 40;
//    case PM_HeaderMargin:return 9;
    case PM_HeaderMargin:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option))
        {
            return 2;
        }
        return 9;
    }

    case PM_MenuBarItemSpacing:return 16;
    case PM_MenuBarVMargin:return 4;
    case PM_ProgressBarChunkWidth: return 9;
    case PM_ToolTipLabelFrameWidth:return 7;
    case PM_MenuButtonIndicator:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            if(tb->subControls & SC_ToolButtonMenu)
                return 16;
        }
        if(const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            return 16;
        }
        return 12;
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
    default:
        break;
    }
    return Style::pixelMetric(metric, option, widget);
}

QRect Qt5UKUIStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        auto rect = Style::subControlRect(control, option, subControl, widget);
        if (subControl == SC_ScrollBarSlider) {
            rect.adjust(1, 1, -1, -1);
            if (option->state.testFlag(QStyle::State_Horizontal)) {
                rect.adjust(1, 0, -1, 0);
            } else {
                rect.adjust(0, 1, 0, -1);
            }
            return rect;
        }
        return rect;
        return scrollBarSubControlRect(control, option, subControl, widget);
    }

    case CC_Slider:
    {
        if(const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option))
        {
            const bool horizontal = slider->state & State_Horizontal;
            int tickOffset = proxy()->pixelMetric(PM_SliderTickmarkOffset, slider, widget);
            int tickGroove = proxy()->pixelMetric(PM_SliderThickness, slider, widget);
            int tickHandle = proxy()->pixelMetric(PM_SliderControlThickness, slider, widget);
            int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
            int sliderPos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                            (horizontal ? slider->rect.width() : slider->rect.height()) - len, slider->upsideDown);
            QRect rect = slider->rect;
            switch (subControl)
            {
            case SC_SliderGroove:
            {
                QRect GrooveRect = slider->rect;
                if (slider->orientation == Qt::Horizontal)
                {
                    rect.setHeight(tickGroove / 4);
                    if (slider->tickPosition & QSlider::TicksAbove)
                        GrooveRect.adjust(0, tickOffset, 0, 0);
                    if (slider->tickPosition & QSlider::TicksBelow)
                        GrooveRect.adjust(0, 0, 0, -tickOffset);
                }
                else
                {
                    rect.setWidth(tickGroove / 4);
                    if (slider->tickPosition & QSlider::TicksLeft)
                        GrooveRect.adjust(tickOffset, 0, 0, 0);
                    if (slider->tickPosition & QSlider::TicksRight)
                        GrooveRect.adjust(0, 0, -tickOffset, 0);
                }
                rect.moveCenter(GrooveRect.center());
                return rect;
            }
            case SC_SliderHandle:
            {
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

    case QStyle::CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            QRect rect = tb->rect;
            qreal width=rect.width();
            qreal mbi = pixelMetric(PM_MenuButtonIndicator, tb, widget);
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, tb, widget);
            qreal js = width - mbi - tb->iconSize.width() - fw*2;
            if(js > 1)
            {
                mbi = qRound(js/2 + mbi);
            }
            if(width < 40)
            {
                mbi = 10;
            }
            else if(width < 30)
            {
                mbi = 8;
            }
            if(mbi > 24)
                mbi = 24;

            switch (subControl) {
            case SC_ToolButton:
                if ((tb->features
                     & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                {
                    rect.adjust(0, 0, -mbi, 0);
                    rect = visualRect(tb->direction,tb->rect,rect);
                }
                return rect;
            case SC_ToolButtonMenu:
                if ((tb->features
                     & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                {
                    rect.adjust(rect.width() -mbi, 0, 0, 0);
                    rect = visualRect(tb->direction,tb->rect,rect);
                }
                return rect;
            default:
                return rect;
            }
        }

    case CC_ComboBox:
    {
        if(const QStyleOptionComboBox* combobox = qstyleoption_cast<const QStyleOptionComboBox*>(option))
        {
            QRect rect = combobox -> rect;
            int fw = combobox->frame ? proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget) : 0;
            const int textMargins = 2*(proxy()->pixelMetric(PM_FocusFrameHMargin) + 1);
            int ArrowWidth = qMax(23, 2*textMargins + proxy()->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget));
            const int gap = combobox->editable ? 4 : 0;

            switch (subControl)
            {
                case SC_ComboBoxFrame:
                    return rect;
                case SC_ComboBoxArrow:
                    rect.setRect(rect.right() - ArrowWidth, rect.top(),ArrowWidth, rect.height());
                    rect = visualRect(combobox->direction,combobox->rect,rect);
                    return rect;
                case SC_ComboBoxEditField:
                    rect.setRect(rect.left()+fw,rect.top()+fw,rect.width() - ArrowWidth - gap - 2*fw,rect.height() - 2*fw);
                    rect = visualRect(combobox->direction,combobox->rect,rect);
                    return rect;
                case SC_ComboBoxListBoxPopup:
                    rect.translate(0,2);
                    rect.adjust(0,2,0,0);
                    return combobox->rect;
                default:
                    break;
            }
        }
        else
        {
            break;
        }
    }

    case CC_SpinBox:
    {
        QSize bs;
        if(const QStyleOptionSpinBox* spinbox = qstyleoption_cast<const QStyleOptionSpinBox*>(option))
        {
            QRect rect;
            int fw = spinbox->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, spinbox, widget) : 0;
            //bs.setHeight(spinbox->rect.height() /2 - fw);
            //bs.setWidth(qMax(20, qMin(bs.height() * 8 / 5, spinbox->rect.width() / 4)));
            bs.setHeight(spinbox->rect.height() /2);
            bs.setWidth(20);
            bs = bs.expandedTo(QApplication::globalStrut());
            //int y = fw + spinbox->rect.y();
            int y = spinbox->rect.y();
            int x, lx, rx;
            //x = spinbox->rect.right() - fw - bs.width();
            x = spinbox->rect.right() - bs.width() + 1;
            lx = fw;
            rx = x - fw;
            switch (subControl)
            {
                case SC_SpinBoxUp:
                    if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                        return QRect();
                    rect = QRect(x, y, bs.width(), bs.height());
                    break;
                case SC_SpinBoxDown:
                    if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                        return QRect();
                    rect = QRect(x, y + bs.height(), bs.width(), bs.height());
                    break;
                case SC_SpinBoxEditField:
                    if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                    {
                        rect = QRect(lx, fw, spinbox->rect.width() - 2*fw, spinbox->rect.height() - 2*fw);
                    }
                    else
                    {
                        rect = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw);
                    }
                    break;
                case SC_SpinBoxFrame:
                    rect = spinbox->rect;
                default:
                    break;
            }
            rect = visualRect(spinbox->direction, spinbox->rect, rect);
            return rect;
        }
        break;
    }
    default:
        break;
    }
    return Style::subControlRect(control, option, subControl, widget);
}

QRect Qt5UKUIStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_HeaderArrow:
    {
        const int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
        QRect rect;
        const int right = option->rect.right();
        const int height = option->rect.height();
        const int top = option->rect.top();
        const int bottom = option->rect.bottom();
        if(option->state & State_Horizontal)
        {
            rect.setRect(right-height+margin,top+margin,option->rect.height()-2*margin,option->rect.height()-2*margin);
        }
        else
        {
            rect.setRect(option->rect.width()+margin,bottom-option->rect.width()+margin,option->rect.width()-2*margin,option->rect.width()-2*margin);
        }
        rect = visualRect(option->direction,option->rect,rect);
        return rect;
    }

    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QRect rect = pb->rect;
            int cw = proxy()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, option, widget);
            if(pb->orientation == Qt::Vertical)
            {
                rect.setRect(pb->rect.left(), pb->rect.top(), cw * 2, rect.height());
            }
            else
            {
                rect.setRect(pb->rect.left(), pb->rect.top(), rect.width(), cw * 2);
            }
            rect.moveCenter(pb->rect.center());
            rect = visualRect(pb->direction, pb->rect, rect);
            return rect;
        }
        break;
    }

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
            newSize += QSize(f->lineWidth * 2, f->lineWidth * 2);
            newSize.setWidth(qMax(newSize.width(), 140));
            newSize.setHeight(qMax(newSize.height(), 32));
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

// This fuction is copied from fusion style.
// The only different is it not hide add/sub line rect
// when style hint SH_ScrollBar_Transient is true.
QRect Qt5UKUIStyle::scrollBarSubControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    QRect ret;
    if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
        const QRect scrollBarRect = scrollbar->rect;
        int sbextent = 0;
        sbextent = proxy()->pixelMetric(PM_ScrollBarExtent, scrollbar, widget);

        int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                          scrollBarRect.width() : scrollBarRect.height()) - (sbextent * 2);
        int sliderlen;

        // calculate slider length
        if (scrollbar->maximum != scrollbar->minimum) {
            uint range = scrollbar->maximum - scrollbar->minimum;
            sliderlen = (qint64(scrollbar->pageStep) * maxlen) / (range + scrollbar->pageStep);

            int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, scrollbar, widget);
            if (sliderlen < slidermin || range > INT_MAX / 2)
                sliderlen = slidermin;
            if (sliderlen > maxlen)
                sliderlen = maxlen;
        } else {
            sliderlen = maxlen;
        }

        int sliderstart = sbextent + sliderPositionFromValue(scrollbar->minimum,
                                                             scrollbar->maximum,
                                                             scrollbar->sliderPosition,
                                                             maxlen - sliderlen,
                                                             scrollbar->upsideDown);

        switch (subControl) {
        case SC_ScrollBarSubLine:            // top/left button
            if (scrollbar->orientation == Qt::Horizontal) {
                int buttonWidth = qMin(scrollBarRect.width() / 2, sbextent);
                ret.setRect(0, 0, buttonWidth, scrollBarRect.height());
            } else {
                int buttonHeight = qMin(scrollBarRect.height() / 2, sbextent);
                ret.setRect(0, 0, scrollBarRect.width(), buttonHeight);
            }
            break;
        case SC_ScrollBarAddLine:            // bottom/right button
            if (scrollbar->orientation == Qt::Horizontal) {
                int buttonWidth = qMin(scrollBarRect.width()/2, sbextent);
                ret.setRect(scrollBarRect.width() - buttonWidth, 0, buttonWidth, scrollBarRect.height());
            } else {
                int buttonHeight = qMin(scrollBarRect.height()/2, sbextent);
                ret.setRect(0, scrollBarRect.height() - buttonHeight, scrollBarRect.width(), buttonHeight);
            }
            break;
        case SC_ScrollBarSubPage:            // between top/left button and slider
            if (scrollbar->orientation == Qt::Horizontal)
                ret.setRect(sbextent, 0, sliderstart - sbextent, scrollBarRect.height());
            else
                ret.setRect(0, sbextent, scrollBarRect.width(), sliderstart - sbextent);
            break;
        case SC_ScrollBarAddPage:            // between bottom/right button and slider
            if (scrollbar->orientation == Qt::Horizontal)
                ret.setRect(sliderstart + sliderlen, 0,
                            maxlen - sliderstart - sliderlen + sbextent, scrollBarRect.height());
            else
                ret.setRect(0, sliderstart + sliderlen, scrollBarRect.width(),
                            maxlen - sliderstart - sliderlen + sbextent);
            break;
        case SC_ScrollBarGroove:
            if (scrollbar->orientation == Qt::Horizontal)
                ret.setRect(sbextent, 0, scrollBarRect.width() - sbextent * 2,
                            scrollBarRect.height());
            else
                ret.setRect(0, sbextent, scrollBarRect.width(),
                            scrollBarRect.height() - sbextent * 2);
            break;
        case SC_ScrollBarSlider:
            if (scrollbar->orientation == Qt::Horizontal)
                ret.setRect(sliderstart, 0, sliderlen, scrollBarRect.height());
            else
                ret.setRect(0, sliderstart, scrollBarRect.width(), sliderlen);
            break;
        default:
            break;
        }
        ret = visualRect(scrollbar->direction, scrollBarRect, ret);
    }
    return ret;
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


void Qt5UKUIStyle::drawComBoxIndicator(SubControl which, const QStyleOptionComplex *option,
                                       QPainter *painter) const
{
    PrimitiveElement arrow=PE_IndicatorArrowDown;
    QRect buttonRect=option->rect.adjusted(+0,+0,-1,-1);
    buttonRect.translate(buttonRect.width()/2,0);
    buttonRect.setWidth((buttonRect.width()+1)/2);
    QStyleOption buttonOpt(*option);
    painter->save();
    painter->setClipRect(buttonRect,Qt::IntersectClip);
    if(!(option->activeSubControls&which))
        buttonOpt.state&=~(State_MouseOver|State_On|State_Sunken);
    QStyleOption arrowOpt(buttonOpt);
    arrowOpt.rect=subControlRect(CC_ComboBox,option,which).adjusted(+0,+0,-0,+0);
    if(arrowOpt.rect.isValid())
        proxy()->drawPrimitive(arrow,&arrowOpt,painter);
    painter->restore();
}


QRect  Qt5UKUIStyle::centerRect(const QRect &rect, int width, int height) const
{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }
