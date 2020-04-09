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

#include <QPaintDevice>

#include "tab-widget-animation-helper.h"
#include "scrollbar-animation-helper.h"
#include "button-animation-helper.h"
#include "button-animator.h"
#include "box-animation-helper.h"

#include "animator-iface.h"
#include "animation-helper.h"

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
#include <QEvent>
#include <QDebug>
#include <QPixmapCache>
#include <QStyleOptionButton>

#include <QApplication>

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
    if (option->fontMetrics.horizontalAdvance(option->text) <= textRect.width())
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

Qt5UKUIStyle::Qt5UKUIStyle(bool dark, bool useDefault) : QFusionStyle ()
{
    m_is_default_style = useDefault;
    m_use_dark_palette = dark;
    m_tab_animation_helper = new TabWidgetAnimationHelper(this);
    m_scrollbar_animation_helper = new ScrollBarAnimationHelper(this);
    m_button_animation_helper = new ButtonAnimationHelper(this);
    m_combobox_animation_helper = new BoxAnimationHelper(this);
}

const QStringList Qt5UKUIStyle::specialList() const
{
    //use dark palette in default style.
    QStringList l;
    l<<"ukui-menu";
    l<<"ukui-panel";
    l<<"ukui-sidebar";
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
        return true;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return int(true);
    case SH_DialogButtons_DefaultButton:
        return int(true);
    case SH_UnderlineShortcut:
        return true;
    case  SH_ComboBox_Popup:
        return false;
    default:
        break;
    }
    return QFusionStyle::styleHint(hint, option, widget, returnData);
}
void Qt5UKUIStyle::polish(QPalette &palette){
    palette = standardPalette();
    return QFusionStyle::polish(palette);
}

QPalette Qt5UKUIStyle::standardPalette() const
{
    auto palette = QFusionStyle::standardPalette();
    //ukui-light
    QColor  window_bg(231,231,231),
            window_no_bg(233,233,233),
            base_bg(255,255,255),
            base_no_bg(248, 248, 248),
            font_bg(0,0,0),
            font_br_bg(255,255,255),
            font_di_bg(191,191,191),
            button_bg(217,217,217),
            button_ac_bg(107,142,235),
            button_di_bg(233,233,233),
            highlight_bg(61,107,229),
            tip_bg(248,248,248),
            tip_font(22,22,22),
            alternateBase(248,248,248);

    if (m_use_dark_palette || (m_is_default_style && specialList().contains(qAppName()))) {
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
        font_di_bg.setAlphaF(0.1);
        button_bg.setRgb(61,61,65);
        button_ac_bg.setRgb(48,48,51);
        button_di_bg.setRgb(48,48,51);
        highlight_bg.setRgb(61,107,229);
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
    palette.setBrush(QPalette::PlaceholderText,font_di_bg);

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
    QFusionStyle::polish(widget);
    if (auto menu = qobject_cast<QMenu*>(widget)) {
        //widget->setAttribute(Qt::WA_TranslucentBackground);
        HighLightEffect::setMenuIconHighlightEffect(menu);
        //QRegion mask = getRoundedRectRegion(widget->rect(), 10, 10);

        //widget->setMask(mask);
        //do not polish widget with proxy style.
        return;
        //qDebug()<<mask<<"menu mask"<<widget->mask();
    }

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

    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->registerWidget(widget);
        m_button_animation_helper->registerWidget(widget);
    }
    widget->installEventFilter(this);
}

void Qt5UKUIStyle::unpolish(QWidget *widget)
{
    widget->removeEventFilter(this);

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
    QFusionStyle::unpolish(widget);
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
                    tmp.state.setFlag(QStyle::State_MouseOver);
                } else {
                    tmp.state.setFlag(QStyle::State_MouseOver, false);
                }

                tmp.palette.setColor(tmp.palette.currentColorGroup(), QPalette::Highlight, color2);
                return QFusionStyle::drawPrimitive(PE_IndicatorBranch, &tmp, painter, widget);
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

    case PE_IndicatorHeaderArrow: //Here is the arrow drawing of the table box

        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(Qt::NoBrush);
            if(option->state & State_Enabled){
                painter->setPen(QPen(option->palette.foreground().color(), 1.1));
                if (option->state & State_MouseOver) {
                    painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
                }
            }
            else {
                painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
            }
            QPolygon points(4);
            //Add 8 to center vertically
            int x = option->rect.x()+8;
            int y = option->rect.y()+8;

            int w = 8;
            int h =  4;
            x += (option->rect.width() - w) / 2;
            y += (option->rect.height() - h) / 2;
            if (header->sortIndicator & QStyleOptionHeader::SortUp) {
                points[0] = QPoint(x, y);
                points[1] = QPoint(x + w / 2, y + h);
                points[2] = QPoint(x + w / 2, y + h);
                points[3] = QPoint(x + w, y);
            } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
                points[0] = QPoint(x, y + h);
                points[1] = QPoint(x + w / 2, y);
                points[2] = QPoint(x + w / 2, y);
                points[3] = QPoint(x + w, y + h);
            }
            painter->drawLine(points[0],  points[1] );
            painter->drawLine(points[2],  points[3] );
            painter->restore();
            return;
        }

    case PE_PanelButtonCommand://UKUI PushButton style
    {
        if(const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
        {
            auto animator = m_button_animation_helper->animator(widget);
            if(animator == nullptr)
            {
                return QFusionStyle::drawPrimitive(PE_PanelButtonCommand,option,painter,widget);
            }
            if(!(button->state & State_Enabled))
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

             if(!(button->state & State_AutoRaise) && !(button->features & QStyleOptionButton::Flat))
             {
                 painter->save();
                 painter->setPen(Qt::NoPen);
                 painter->setBrush(option->palette.color(QPalette::Button));
                 painter->setRenderHint(QPainter::Antialiasing,true);
                 painter->drawRoundedRect(option->rect,4,4);
                 painter->restore();
             }
            if((button->state & (State_Sunken | State_On)) || animator->isRunning("SunKen")
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
                if (animator->isRunning("SunKen"))
                {
                    const_cast<QWidget *>(widget)->update();
                }
                painter->save();
                auto color = button->palette.color(QPalette::Highlight).lighter(125);
                painter->setBrush(color);
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->drawRoundedRect(button->rect,4,4);
                painter->restore();

                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(button->palette.color(QPalette::Highlight));
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
                if (animator->isRunning("MouseOver"))
                {
                    const_cast<QWidget *>(widget)->update();
                }
                painter->save();
                painter->setOpacity(opacity);
                auto color = button->palette.color(QPalette::Highlight).lighter(125);
                painter->setBrush(color);
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
        //        painter->save();
        //        painter->setRenderHint(QPainter::Antialiasing,true);
        //        /*!
        //       * \todo
        //       * It is selected by default and not done first,because it conflicts with the normal button.
        //       */
        //        //        if(option->state & State_HasFocus){
        //        //            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Button));
        //        //            painter->setBrush(option->palette.color(QPalette::Highlight));
        //        //        }
        //        //        else {
        //        painter->setPen(Qt::NoPen);
        //        painter->setBrush(option->palette.color(QPalette::Button));
        //        //        }

        //        if (option->state & State_MouseOver) {
        //            if (option->state & State_Sunken) {
        //                painter->setPen(Qt::NoPen);
        //                painter->setBrush(option->palette.color(QPalette::Highlight));
        //            } else {
        //                painter->setPen(Qt::NoPen);
        //                painter->setBrush(option->palette.color(QPalette::Highlight));
        //            }
        //        }
        //        painter->drawRoundedRect(option->rect,4,4);
        //        painter->restore();

        //        /*!
        //       * \todo
        //       * Judge whether it is OK or other buttons
        //       */

        //        return;
        //        }

    case PE_PanelTipLabel://UKUI Tip  style: Open ground glass
        {
            if (widget->isEnabled()) {
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
            return QFusionStyle::drawPrimitive(element, option, painter, widget);
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
        if(widget->isEnabled()){
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
        if(animator == nullptr)
            return QFusionStyle::drawPrimitive(PE_PanelButtonTool,option,painter,widget);
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

            if (animator->isRunning("SunKen"))
            {
                const_cast<QWidget *>(widget)->update();
            }
            painter->save();
            auto color = option->palette.color(QPalette::Highlight).lighter(125);
            painter->setBrush(color);
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->drawRoundedRect(option->rect,4,4);
            painter->restore();

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(option->palette.color(QPalette::Highlight));
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
            if (animator->isRunning("MouseOver"))
            {
                const_cast<QWidget *>(widget)->update();
            }
            painter->save();
            painter->setOpacity(opacity);
            auto color = option->palette.color(QPalette::Highlight).lighter(125);
            painter->setBrush(color);
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
                QFusionStyle::drawPrimitive(element, option, painter, widget);
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
        // Conflict with qspinbox and so on, The widget text cannot use this style
        if (widget) {
            if (widget->parentWidget())
                if (widget->parentWidget()->inherits("QDoubleSpinBox")|widget->parentWidget()->inherits("QSpinBox")|widget->parentWidget()->inherits("QComboBox")) {
                    return;
                }
        }
        painter->save();
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->setRenderHint(QPainter::Antialiasing,true);
            //Setpen is set to avoid a bug that collides with a white background
            painter->setPen(option->palette.color(QPalette::Window));
            painter->setBrush(option->palette.color(QPalette::Base));
            // if (panel->lineWidth > 0)
            // proxy()->drawPrimitive(PE_FrameLineEdit, panel, painter, widget);
            if (widget->isEnabled()) {
                if (option->state &State_MouseOver) {
                    painter->setBrush(option->palette.button().color().lighter());
                }
                if(option->state &State_HasFocus) {
                    painter->setPen(option->palette.color(QPalette::Highlight));
                    painter->setBrush(option->palette.color(QPalette::Base));
                }
            }
            painter->drawRoundedRect(panel->rect,4,4);
        }
        painter->restore();
        return;
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
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft:
    {
        if (option->rect.width() <= 1 || option->rect.height() <= 1)
            break;
        QRect r = option->rect;
        int size = qMin(r.height(), r.width());
        QPixmap pixmap;
        QString key;
        int width = option->rect.width();
        int height = option->rect.height();
        int state = option->state;
        int direction = element;
        while (width > 0) {
            char a = width % 10 + 48;
            key.insert(0,&a);
            width /= 10;
        }
        while (height > 0) {
            char a = height % 10 + 48;
            key.insert(0,&a);
            height /= 10;
        }
        while (state > 0) {
            char a = state % 10 + 48;
            key.insert(0,&a);
            state /= 10;
        }
        while (direction > 0)
        {
            char a = direction % 10 + 48;
            key.insert(0,&a);
            direction /= 10;
        }
        qreal pixelRatio = painter->device()->devicePixelRatioF();
        int border = qRound(pixelRatio*(size/4));
        int sqsize = qRound(pixelRatio*(2*(size/2)));
        if(size > 16)
        {
            border = pixelRatio*4;
            sqsize = pixelRatio*16;
        }
//        if (!QPixmapCache::find(key, pixmap)) {

//            QPixmapCache::insert(key, pixmap);
//        }
        QImage image(sqsize, sqsize, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter imagePainter(&image);
        int sx = 0;
        int sy = (sqsize/2 - border)/2;
        QLineF lines[2];
        switch (element) {
        case PE_IndicatorArrowUp:
            lines[0] = QLine(border, sqsize/2, sqsize/2, border);
            lines[1] = QLine(sqsize/2, border, sqsize - border, sqsize/2);
            break;
        case PE_IndicatorArrowDown:
            lines[0] = QLine(border, border, sqsize/2, sqsize/2);
            lines[1] = QLine(sqsize/2, sqsize/2, sqsize - border, border);
            break;
        case PE_IndicatorArrowRight:
            lines[0] = QLine(border, border, sqsize/2, sqsize/2);
            lines[1] = QLine(sqsize/2, sqsize/2, border, sqsize - border);
            sx = (sqsize/2 - border)/2;
            sy = 0;
            break;
        case PE_IndicatorArrowLeft:
            lines[0] = QLine(sqsize/2, border, border, sqsize/2);
            lines[1] = QLine(border, sqsize/2, sqsize/2, sqsize - border);
            sx = (sqsize/2 - border)/2;
            sy = 0;
            break;
        default:
            break;
        }
        imagePainter.translate(sx , sy);
        imagePainter.setPen(Qt::NoPen);
        imagePainter.setPen(QPen(option->palette.foreground().color(), 1.1));
        if (option->state & (State_MouseOver|State_Sunken))
        {
            imagePainter.setPen(QPen(option->palette.color(QPalette::Light), 1.1));
        }
        imagePainter.setBrush(Qt::NoBrush);
        imagePainter.setRenderHint(QPainter::Qt4CompatiblePainting);
        imagePainter.setRenderHint(QPainter::Antialiasing);

        if (!(option->state & State_Enabled)) {
            //imagePainter.translate(1, 1);
            imagePainter.setPen(QPen(option->palette.color(QPalette::Disabled,QPalette::WindowText), 1.3));
            imagePainter.setBrush(Qt::NoBrush);
            imagePainter.drawLines(lines,2);
            //imagePainter.translate(-1, -1);
            //imagePainter.setBrush(option->palette.mid().color());
            //imagePainter.setPen(option->palette.mid().color());
        }
        else
        {
            imagePainter.drawLines(lines,2);
        }
        imagePainter.end();
        pixmap = QPixmap::fromImage(image);
        pixmap.setDevicePixelRatio(pixelRatio);
        int xOffset = r.x() + (r.width() - sqsize)/2;
        int yOffset = r.y() + (r.height() - sqsize)/2;
        painter->drawPixmap(xOffset, yOffset, pixmap);
        return;
    }

    case PE_IndicatorRadioButton:{
        auto radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option);

        if (option->state & State_None){//Non optional status
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Disabled,QPalette::Button));
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Mid));
            painter->drawEllipse(option->rect.x()+1,option->rect.y(), 16.0, 16.0);
            painter->restore();
        }
        else if (option->state & State_Off) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Button));
            painter->setPen(option->palette.color(QPalette::Mid));
            if (option->state & State_Sunken) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }else if (option->state & State_MouseOver){
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }
            painter->drawEllipse(option->rect.x()+1,option->rect.y(), 16.0, 16.0);
            painter->restore();
        }
        else if (option->state & State_On) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Highlight));
            painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            if (option->state & State_Sunken) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }else if(option->state & State_MouseOver){
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }
            painter->drawEllipse(option->rect.x()+1,option->rect.y(), 16.0, 16.0);
            painter->restore();

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(option->palette.color(QPalette::HighlightedText));
            painter->setBrush(option->palette.color(QPalette::HighlightedText));
            painter->drawEllipse(option->rect.x()+6, option->rect.y()+5, 6.0, 6.0);
            painter->restore();
        }
        return;
    }


    default:   break;
    }
    return QFusionStyle::drawPrimitive(element, option, painter, widget);
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
            return QFusionStyle::drawComplexControl(control, option, painter, widget);
        }

        animator->setAnimatorDirectionForward("bg_opacity", mouse_over);
        animator->setAnimatorDirectionForward("groove_width", mouse_over);
        if (enable) {
            if (mouse_over) {
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") < animator->totalAnimationDuration("groove_width")) {
                    animator->startAnimator("bg_opacity");
                    animator->startAnimator("groove_width");
                }
            } else {
                if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") > 0) {
                    animator->startAnimator("groove_width");
                    animator->startAnimator("bg_opacity");
                }
            }
        }

        if (animator->isRunning("groove_width")) {
            const_cast<QWidget*>(widget)->update();
        }

        painter->save();
        painter->setPen(Qt::transparent);
        painter->setBrush(tmp.palette.windowText());
        auto percent = animator->value("groove_width").toInt()*1.0/12;
        painter->setOpacity(0.1*percent);
        auto grooveRect = option->rect;
        auto currentWidth = animator->value("groove_width").toInt();
        if (is_horizontal) {
            grooveRect.setY(qMax(grooveRect.height() - currentWidth*2, 0));
        } else {
            grooveRect.setX(qMax(grooveRect.width() - currentWidth*2, 0));
        }
        if (widget->property("drawScrollBarGroove").isValid()) {
            if (!widget->property("drawScrollBarGroove").toBool()) {
                painter->restore();
                return QCommonStyle::drawComplexControl(control, option, painter, widget);
            }
        }
        painter->drawRect(grooveRect);
        painter->restore();

        return QCommonStyle::drawComplexControl(control, option, painter, widget);
    }

    case CC_ComboBox:
    {
        if(const QStyleOptionComboBox* combobox = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            auto animator = m_combobox_animation_helper->animator(widget);
            if(animator == nullptr)
              return QFusionStyle::drawComplexControl(CC_ComboBox,option,painter,widget);

            QRect rect = subControlRect(CC_ComboBox,option,SC_ComboBoxFrame,widget);
            QRect ArrowRect = subControlRect(CC_ComboBox,option,SC_ComboBoxArrow,widget);
            QStyleOptionButton button;
            button.state = combobox->state;
            button.rect = ArrowRect;
            if(combobox->editable)
            {
                proxy()->drawPrimitive(PE_PanelButtonCommand,&button,painter,widget);
                QRect EditRect = subControlRect(CC_ComboBox,option,SC_ComboBoxEditField,widget);
                rect = EditRect;
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
            proxy()->drawPrimitive(PE_IndicatorArrowDown,&button,painter,widget);


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
              if (animator->isRunning("SunKen"))
              {
                  const_cast<QWidget *>(widget)->update();
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
              if (animator->isRunning("MouseOver"))
              {
                  const_cast<QWidget *>(widget)->update();
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

    case CC_SpinBox:
    {
        const QStyleOptionSpinBox *pb=qstyleoption_cast<const QStyleOptionSpinBox*>(option);
        QRectF r1=subControlRect(control,option,QStyle::SC_SpinBoxUp,widget);
        QRectF r2=subControlRect(control,option,QStyle::SC_SpinBoxDown,widget);
        // QRect r3=subControlRect(control,option,QStyle::SC_SpinBoxEditField,widget);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(QPen(option->palette.color(QPalette::Button),1));
        painter->setBrush(option->palette.color(QPalette::Button));
        if (widget->isEnabled()) {
            if(pb->state&QStyle::State_HasFocus){
                painter->setPen(QPen(option->palette.color(QPalette::Highlight),1));
            }

            if(pb->state&State_MouseOver){
                painter->setPen(option->palette.color(QPalette::Highlight));
            }
        }
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();

        /*
         * There's no PE_IndicatorSpinUp and PE_IndicatorSpinDown here, and it's drawn directly.
        */
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setBrush(Qt::NoBrush);
        if(option->state & State_Enabled){
            painter->setPen(QPen(option->palette.foreground().color(), 1.1));
            if (option->state & State_MouseOver) {
                painter->restore();
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));
            }
        }
        else {
            painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
        }
        painter->fillRect(int(r1.x())-2, int(r1.y()), int(r1.width()), int(r1.height()+r2.height()),Qt::NoBrush);

        int w = 8;
        int h =  4;

        QPolygon points(4);
        int x = int(r1.x())+2;
        int y = int(r1.y())+2;
        points[0] = QPoint(x, y + h);
        points[1] = QPoint(x + w / 2, y);
        points[2] = QPoint(x + w / 2, y);
        points[3] = QPoint(x + w, y + h);
        painter->drawLine(points[0],  points[1] );
        painter->drawLine(points[2],  points[3] );

        int x2 = int(r2.x())+2;
        int y2 = int(r2.y())+2;
        points[0] = QPoint(x2, y2);
        points[1] = QPoint(x2 + w / 2, y2 + h);
        points[2] = QPoint(x2 + w / 2, y2 + h);
        points[3] = QPoint(x2 + w, y2);
        painter->drawLine(points[0],  points[1] );
        painter->drawLine(points[2],  points[3] );
        painter->restore();

        return ;
    }

    case CC_Slider :
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            //Size and location of each rectangle used
            QRectF rect = option->rect;
            QRectF rectHandle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            QRectF rectSliderTickmarks = proxy()->subControlRect(CC_Slider, option, SC_SliderTickmarks, widget);
            QRect rectGroove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QPen pen;
            //Drawing chute (line)
            if (option->subControls & SC_SliderGroove) {
                pen.setStyle(Qt::CustomDashLine);
                QVector<qreal> dashes;
                //qreal space = 1.3;
                qreal space = 0;
                dashes << 0.1 << space;
                // dashes << -0.1 << space;
                pen.setDashPattern(dashes);
                pen.setWidthF(3);
                pen.setColor(option->palette.highlight().color().lighter());
                painter->setPen(pen);
                painter->setRenderHint(QPainter::Antialiasing);

                if (slider->orientation == Qt::Horizontal) {
                    painter->drawLine(QPointF(rectGroove.left(), rectHandle.center().y()), QPointF(rectHandle.left(), rectHandle.center().y()));
                    pen.setColor(option->palette.color(QPalette::Button));
                    painter->setPen(pen);
                    painter->drawLine(QPointF(rectGroove.right(), rectHandle.center().y()), QPointF(rectHandle.right(), rectHandle.center().y()));
                } else {
                    painter->drawLine(QPointF(rectGroove.center().x(), rectGroove.bottom()), QPointF(rectGroove.center().x(),  rectHandle.bottom()));
                    pen.setColor(option->palette.color(QPalette::Button));
                    painter->setPen(pen);
                    painter->drawLine(QPointF(rectGroove.center().x(),  rectGroove.top()), QPointF(rectGroove.center().x(),  rectHandle.top()));
                }
            }

            //Painting slider
            if (option->subControls & SC_SliderHandle) {
                pen.setStyle(Qt::SolidLine);
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->drawEllipse(rectHandle);
            }

            //Drawing scale
            if ((option->subControls & SC_SliderTickmarks) && slider->tickInterval) {
                painter->setPen(option->palette.foreground().color());
                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                //int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);
                //int ticks = slider->tickPosition;
                int v = slider->minimum;
                int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
                while (v <= slider->maximum + 1) {
                    const int v_ = qMin(v, slider->maximum);
                    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, v_, available) + len / 2;

                    if (slider->orientation == Qt::Horizontal) {
                        if (slider->tickPosition == QSlider::TicksBothSides) {
                            painter->drawLine(pos, int(rect.top()), pos, int(rectHandle.top()));
                            painter->drawLine(pos, int(rect.bottom()), pos, int(rectHandle.bottom()));
                        } else {
                            painter->drawLine(pos, int(rectSliderTickmarks.top()), pos, int(rectSliderTickmarks.bottom()));
                        }
                    } else {
                        if (slider->tickPosition == QSlider::TicksBothSides) {
                            painter->drawLine(int(rect.left()), pos, int(rectHandle.left()), pos);
                            painter->drawLine(int(rect.right()), pos, int(rectHandle.right()), pos);
                        } else {
                            painter->drawLine(int(rectSliderTickmarks.left()), pos, int(rectSliderTickmarks.right()), pos);
                        }
                    }
                    // in the case where maximum is max int
                    int nextInterval = v + interval;
                    if (nextInterval < v)
                        break;
                    v = nextInterval;
                }
            }
            return;
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
                if (toolbutton->activeSubControls & SC_ToolButton)
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

    default:        return QFusionStyle::drawComplexControl(control, option, painter, widget);
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

            auto target = pixmap;

            if (widget) {
                if (widget->property("useIconHighlightEffect").isValid()) {
                    bool needHandel = widget->property("useIconHighlightEffect").toBool();
                    if (needHandel) {
                        HighLightEffect::EffectMode mode = HighLightEffect::HighlightOnly;
                        if (widget->property("iconHighlightEffectMode").isValid()) {
                            auto var = widget->property("iconHighlightEffectMode");
                            mode = qvariant_cast<HighLightEffect::EffectMode>(var);
                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false, mode);
                        } else {
                            target = HighLightEffect::generatePixmap(pixmap, vopt, widget, false);
                        }
                    }
                }
            }
            QFusionStyle::drawItemPixmap(painter, iconRect, vopt->decorationAlignment, target);

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
    case CE_ProgressBarGroove:{
        const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
        if (!bar)
            return;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.button());
        int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
        bool needAdjustBarWidth = adjustMarignx2 > 0;
        bool vertical = (bar->orientation == Qt::Vertical);
        auto progressBarGroveRect = option->rect;
        if (!vertical) {
            if (needAdjustBarWidth) {
                progressBarGroveRect.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
            }
        } else {
            if (needAdjustBarWidth) {
                progressBarGroveRect.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
            }
        }
        painter->drawRoundedRect(progressBarGroveRect, 4, 4);
        painter->restore();
        return;
    }
    case CE_ProgressBarContents:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        if (const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            //Judgment status
            painter->setPen(Qt::NoPen);
            bool vertical = false;
            bool inverted = false;
            bool indeterminate = (bar->minimum == 0 && bar->maximum == 0);
            bool complete = bar->progress == bar->maximum;

            // Get extra style options if version 2
            vertical = (bar->orientation == Qt::Vertical);
            inverted = bar->invertedAppearance;

            // If the orientation is vertical, we use a transform to rotate
            // the progress bar 90 degrees clockwise.  This way we can use the
            // same rendering code for both orientations.
            int maxWidth = vertical? option->rect.height(): option->rect.width();
            const auto progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
            const auto totalSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
            const auto progressSteps = qint64(progress) - bar->minimum;
            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
            int width = indeterminate ? maxWidth : progressBarWidth;

            QRect progressBar;
            painter->setPen(Qt::NoPen);


            int adjustMarignx2 = qMin(option->rect.width(), option->rect.height()) - 16;
            bool needAdjustBarWidth = adjustMarignx2 > 0;

            //Positioning
            progressBar = option->rect;
            if (!indeterminate) {
                if (!inverted) {
                    if (!vertical) {
                        //progressBar.setRect(option->rect.left()-1, option->rect.top(), width-3,option->rect.height()-7);
                        progressBar.setRight(width);
                        if (needAdjustBarWidth) {
                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
                        }
                    } else {
                        progressBar.setTop(maxWidth - width);
                        if (needAdjustBarWidth) {
                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
                        }
                    }
                } else {
                    if (!vertical) {
                        progressBar = option->rect;
                        progressBar.setLeft(maxWidth - width);
                        if (needAdjustBarWidth) {
                            progressBar.adjust(0, adjustMarignx2/2, 0, -adjustMarignx2/2);
                        }
                    } else {
                        progressBar = option->rect;
                        progressBar.setBottom(width);
                        if (needAdjustBarWidth) {
                            progressBar.adjust(adjustMarignx2/2, 0, -adjustMarignx2/2, 0);
                        }
                    }
                }
            }

            //Brush color
            if (!indeterminate && width > 0) {
                painter->save();
                if (!vertical) {
                    painter->setPen(Qt::NoPen);

                    QColor startcolor = option->palette.highlight().color();
                    QColor endcolor = option->palette.highlight().color().darker(200);
                    QLinearGradient linearGradient(QPoint(option->rect.bottomRight().x(), option->rect.bottomRight().y()),
                                                   QPoint(option->rect.bottomLeft().x(), option->rect.bottomLeft().y()));
                    linearGradient.setColorAt(1,startcolor);
                    linearGradient.setColorAt(0,endcolor);
                    painter->setBrush(QBrush(linearGradient));

                    // painter->setBrush(option->palette.highlight().color());

                    if (!complete && !indeterminate)
                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

                    painter->drawRoundedRect(progressBar, 4, 4);
                } else {
                    painter->setPen(Qt::NoPen);

                    QColor startcolor = option->palette.highlight().color();
                    QColor endcolor = option->palette.highlight().color().darker(200);
                    QLinearGradient linearGradient(QPoint(option->rect.topLeft()),
                                                   QPoint(option->rect.bottomLeft()));
                    linearGradient.setColorAt(0,startcolor);
                    linearGradient.setColorAt(1,endcolor);
                    painter->setBrush(QBrush(linearGradient));

                    if (!complete && !indeterminate)
                        painter->setClipRect(progressBar.adjusted(0, 0, 0, 0));

                    painter->drawRoundedRect(progressBar, 4, 4);
                }
                painter->restore();
            } else {
                //FIXME: implement waiting animation.
                //painter->fillRect(option->rect, Qt::red);
            }
        }
        painter->restore();
        return;
    }
    case CE_ProgressBarLabel:{
        return;
    }
    case CE_ScrollBarSlider: {
        //qDebug()<<"draw slider";
        //auto animatorObj = widget->findChild<QObject*>("ukui_scrollbar_default_interaction_animator");
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return QFusionStyle::drawControl(element, option, painter, widget);
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
            auto sliderWidth = 0;
            if (is_horizontal) {
                sliderWidth = qMin(animator->value("groove_width").toInt() + 4, option->rect.height());
            } else {
                sliderWidth = qMin(animator->value("groove_width").toInt() + 4, option->rect.width());
            }

            animator->setAnimatorDirectionForward("slider_opacity", mouse_over);
            if (mouse_over) {
                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") == 0) {
                    animator->startAnimator("slider_opacity");
                }
            } else {
                animator->setAnimatorDirectionForward("additional_opacity", false);
                //                if (animator->currentAnimatorTime("slider_opacity") > 0)
                //                    animator->startAnimator("additional_opacity");

                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") > 0) {
                    animator->startAnimator("slider_opacity");
                }
            }

            //sunken additional opacity

            if (is_sunken) {
                if (animator->currentAnimatorTime("additional_opacity") == 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", is_sunken);
                    animator->startAnimator("additional_opacity");
                    //qDebug()<<"start is_sunken";
                }
            } else {
                if (animator->currentAnimatorTime("additional_opacity") > 0) {
                    animator->setAnimatorDirectionForward("additional_opacity", is_sunken);
                    animator->startAnimator("additional_opacity");
                    //qDebug()<<"start not_is_sunken";
                }
            }

            if (animator->isRunning("additional_opacity") || animator->isRunning("slider_opacity")) {
                const_cast<QWidget *>(widget)->update();
                //qDebug()<<"sunken"<<is_sunken<<animator->value("additional_opacity").toDouble();
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
                sliderRect.setY(sliderRect.height() - sliderWidth);
            } else {
                sliderRect.setX(sliderRect.width() - sliderWidth);
            }
            if (sliderWidth > 4) {
                if (is_horizontal) {
                    sliderRect.adjust(0, 1, 0, -1);
                } else {
                    sliderRect.adjust(1, 0, -1, 0);
                }
            } else {
                //                if (is_horizontal) {
                //                    sliderRect.adjust(0, -1, 0, -1);
                //                } else {
                //                    sliderRect.adjust(-1, 0, -1, 0);
                //                }
            }
            int rectMin = qMin(sliderRect.width(), sliderRect.height());
            painter->drawRoundedRect(sliderRect, rectMin/2, rectMin/2);
            painter->restore();
        }
        return;
    }
    case CE_ScrollBarAddLine: {
        //auto animatorObj = widget->findChild<QObject*>("ukui_scrollbar_default_interaction_animator");
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return QFusionStyle::drawControl(element, option, painter, widget);
        }

        painter->save();
        auto percent = animator->value("groove_width").toInt()*1.0/12;
        painter->setOpacity(percent);
        //QFusionStyle::drawControl(element, option, painter, widget);

        QIcon icon;
        if (option->state.testFlag(State_Horizontal)) {
            icon = QIcon::fromTheme("pan-end-symbolic");
        } else {
            icon = QIcon::fromTheme("pan-down-symbolic");
        }
        icon.paint(painter, option->rect, Qt::AlignCenter);

        painter->restore();
        return;
    }
    case CE_ScrollBarSubLine: {
        //auto animatorObj = widget->findChild<QObject*>("ukui_scrollbar_default_interaction_animator");
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return QFusionStyle::drawControl(element, option, painter, widget);
        }

        painter->save();
        auto percent = animator->value("groove_width").toInt()*1.0/12;
        painter->setOpacity(percent);
        //QFusionStyle::drawControl(element, option, painter, widget);

        QIcon icon;
        if (option->state.testFlag(State_Horizontal)) {
            icon = QIcon::fromTheme("pan-start-symbolic");
        } else {
            icon = QIcon::fromTheme("pan-up-symbolic");
        }
        icon.paint(painter, option->rect, Qt::AlignCenter);

        painter->restore();
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
    {
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
        {
            bool enabled = cb->state & State_Enabled;
            QRect editRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
            painter->save();
            painter->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
               QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal: QIcon::Disabled;
               QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
               QRect iconRect(editRect);
               iconRect.setWidth(cb->iconSize.width() + 4);
               iconRect = alignedRect(cb->direction,Qt::AlignLeft | Qt::AlignVCenter,iconRect.size(), editRect);
               iconRect.adjust(1,1,-1,-1);
               if (cb->editable)
                   painter->fillRect(iconRect,
                                     cb->palette.brush(enabled ? QPalette::Active: QPalette::Disabled,QPalette::Button));
               proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

               if (cb->direction == Qt::RightToLeft)
                   editRect.translate(-4 - cb->iconSize.width(), 0);
               else
                   editRect.translate(cb->iconSize.width() + 4, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable)
            {
               proxy()->drawItemText(painter, editRect.adjusted(1, 0, -1, 0),visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                     cb->palette, cb->state & State_Enabled, cb->currentText,QPalette::ButtonText);
            }
            painter->restore();
        }
        break;
    }


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
            bool isRadio = (element == CE_RadioButton);
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect( SE_RadioButtonIndicator, btn, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton ,&subopt, painter, widget);
            subopt.rect = subElementRect( SE_RadioButtonContents, btn, widget);
            proxy()->drawControl( CE_RadioButtonLabel , &subopt, painter, widget);
            if (btn->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = subElementRect(SE_RadioButtonFocusRect, btn, widget);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
            }
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
    case CE_HeaderSection:
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillRect(option->rect, option->palette.alternateBase().color());
        painter->restore();
        return;
    }break;


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
                if (!QFusionStyle::styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                proxy()->drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            } else {

            }
        }
        painter->restore();

        return;
        break;

    }

        //Draw submenu style
    case CE_MenuItem:
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        // Draws one item in a popup menu.
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            // QColor highlightOutline = highlightedOutline;
            QColor highlightOutline = option->palette.highlight().color();
            QColor highlight = option->palette.highlight().color();
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                int w = 0;
                // const int margin = QStyleHelper::dpiScaled(5);
                const int margin =5;
                if (!menuItem->text.isEmpty()) {
                    painter->setFont(menuItem->font);
                    proxy()->drawItemText(painter, menuItem->rect.adjusted(margin, 0, -margin, 0), Qt::AlignLeft | Qt::AlignVCenter,
                                          menuItem->palette, menuItem->state & State_Enabled, menuItem->text,
                                          QPalette::Text);
                    w = menuItem->fontMetrics.horizontalAdvance(menuItem->text) + margin;
                }
                // painter->setPen(shadow.lighter(106));
                painter->setPen(option->palette.color(QPalette::Disabled,QPalette::WindowText));
                painter->setOpacity(0.9);
                bool reverse = menuItem->direction == Qt::RightToLeft;
                painter->drawLine(menuItem->rect.left() + margin + (reverse ? 0 : w), menuItem->rect.center().y(),
                                  menuItem->rect.right() - margin - (reverse ? w : 0), menuItem->rect.center().y());
                painter->restore();
                break;
            }
            bool selected = menuItem->state & State_Selected && menuItem->state & State_Enabled;
            if (selected) {
                QRect r = option->rect;
                painter->setBrush(highlightOutline);
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(r.adjusted(2, 0.5, -2, -0.5),4,4);
            }
            bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
            bool checked = menuItem->checked;
            bool sunken = menuItem->state & State_Sunken;
            bool enabled = menuItem->state & State_Enabled;

            bool ignoreCheckMark = false;
            //  const int checkColHOffset = windowsItemHMargin + windowsItemFrame - 1;
            //  int checkcol = qMax<int>(menuItem->rect.height() * 0.79,qMax<int>(menuItem->maxIconWidth, dpiScaled(21))); // icon checkbox's highlight column width
            const int checkColHOffset = 5;
            int checkcol = qMax<int>(menuItem->rect.height() * 0.79,qMax<int>(menuItem->maxIconWidth, 21));
            if (qobject_cast<const QComboBox* >(widget) ||(option->styleObject && option->styleObject->property("_q_isComboBoxPopupItem").toBool()))
                ignoreCheckMark = true; //ignore the checkmarks provided by the QComboMenuDelegate

            if (!ignoreCheckMark) {
                // Check, using qreal and QRectF to avoid error accumulation
                //const qreal boxMargin = dpiScaled(3.5);
                const qreal boxMargin = 3.5;
                const qreal boxWidth = checkcol - 2 * boxMargin;
                QRectF checkRectF(option->rect.left() + boxMargin + checkColHOffset, option->rect.center().y() - boxWidth/2 + 1, boxWidth, boxWidth);
                QRect checkRect = checkRectF.toRect();
                checkRect.setWidth(checkRect.height()); // avoid .toRect() round error results in non-perfect square
                checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);
                if (checkable) {
                    if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
                        // Radio button
                        if (checked || sunken) {
                            painter->setRenderHint(QPainter::Antialiasing);
                            painter->setPen(Qt::NoPen);

                            QPalette::ColorRole textRole = !enabled ? QPalette::Text:
                                                                      selected ? QPalette::HighlightedText : QPalette::ButtonText;
                            painter->setBrush(option->palette.brush( option->palette.currentColorGroup(), textRole));
                            const int adjustment = checkRect.height() * 0.3;
                            painter->drawEllipse(checkRect.adjusted(adjustment, adjustment, -adjustment, -adjustment));
                        }
                    } else {
                        // Check box
                        if (menuItem->icon.isNull()) {
                            QStyleOptionButton box;
                            box.QStyleOption::operator=(*option);
                            box.rect = checkRect;
                            if (checked)
                                box.state |= State_On;
                            proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
                        }
                    }
                }
            } else { //ignore checkmark
                if (menuItem->icon.isNull())
                    checkcol = 0;
                else
                    checkcol = menuItem->maxIconWidth;
            }

            // Text and icon, ripped from windows style
            bool dis = !(menuItem->state & State_Enabled);
            bool act = menuItem->state & State_Selected;
            const QStyleOption *opt = option;
            const QStyleOptionMenuItem *menuitem = menuItem;

            QPainter *p = painter;
            QRect vCheckRect = visualRect(opt->direction, menuitem->rect,
                                          QRect(menuitem->rect.x() + checkColHOffset, menuitem->rect.y(),
                                                checkcol, menuitem->rect.height()));
            if (!menuItem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;

                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                QSize iconSize(smallIconSize, smallIconSize);

                if (const QComboBox *combo = qobject_cast<const QComboBox*>(widget))
                    iconSize = combo->iconSize();

                if (checked)
                    pixmap = menuItem->icon.pixmap(iconSize, mode, QIcon::On);
                else
                    pixmap = menuItem->icon.pixmap(iconSize, mode);

                const int pixw = pixmap.width() / pixmap.devicePixelRatio();
                const int pixh = pixmap.height() / pixmap.devicePixelRatio();

                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                painter->setPen(menuItem->palette.text().color());
                if (!ignoreCheckMark && checkable && checked) {
                    QStyleOption opt = *option;
                    if (act) {
                        //                            QColor activeColor = mergedColors(option->palette.background().color(),
                        //                                                              option->palette.highlight().color());
                        QColor activeColor =option->palette.background().color();
                        opt.palette.setBrush(QPalette::Button, activeColor);
                    }
                    opt.state |= State_Sunken;
                    opt.rect = vCheckRect;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &opt, painter, widget);
                }
                //FIXME: add highlight effect support.

                QPixmap target = HighLightEffect::generatePixmap(pixmap, menuItem, widget);
                painter->drawPixmap(pmr.topLeft(), target);
            }
            if (selected) {
                painter->setPen(menuItem->palette.highlightedText().color());
            } else {
                painter->setPen(menuItem->palette.text().color());
            }
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }
            //int xm = checkColHOffset + checkcol + windowsItemHMargin;
            int xm = checkColHOffset + checkcol + 5;
            int xpos = menuitem->rect.x() + xm;

            //                QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect textRect(xpos, y + 5, w - xm - 5 - tab + 1, h - 2 * 5);

            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QStringRef s(&menuitem->text);
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf(QLatin1Char('\t'));
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!QFusionStyle::styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect,
                                                     QRect(textRect.topRight(), QPoint(menuitem->rect.right(), textRect.bottom())));
                    const QString textToDraw = s.mid(t + 1).toString();
                    if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, option, widget)) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1, 1, 1, 1), text_flags, textToDraw);
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, textToDraw);
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                // font may not have any "hard" flags set. We override
                // the point size so that when it is resolved against the device, this font will win.
                // This is mainly to handle cases where someone sets the font on the window
                // and then the combo inherits it and passes it onward. At that point the resolve mask
                // is very, very weak. This makes it stonger.
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());

                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);

                p->setFont(font);
                const QString textToDraw = s.left(t).toString();
                if (dis && !act && proxy()->styleHint(SH_EtchDisabledText, option, widget)) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1, 1, 1, 1), text_flags, textToDraw);
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, textToDraw);
                p->restore();
            }

            // Arrow
            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (menuItem->rect.height() - 4) / 2;
                PrimitiveElement arrow;
                arrow = option->direction == Qt::RightToLeft ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                int xpos = menuItem->rect.left() + menuItem->rect.width() - 3 - dim;
                QRect  vSubMenuRect = visualRect(option->direction, menuItem->rect,
                                                 QRect(xpos, menuItem->rect.top() + menuItem->rect.height() / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuItem;
                newMI.rect = vSubMenuRect;
                newMI.state = !enabled ? State_None : State_Enabled;
                if (selected)
                    newMI.palette.setColor(QPalette::Foreground,
                                           newMI.palette.highlightedText().color());
                proxy()->drawPrimitive(arrow, &newMI, painter, widget);
            }
        }
        painter->restore();
        return;
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
        return QFusionStyle::drawControl(element, option, painter, widget);
    }
}

int Qt5UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent: {
        return 10;
    }
    case PM_ScrollView_ScrollBarOverlap: {
        return -10;
    }
    case PM_SliderThickness:{
        return 40;
    }
    case PM_SliderLength:{
        return 20;
    }
    case PM_MenuHMargin:{
        return 5;
    }
    case PM_MenuVMargin:{
        return 10;
    }
    case PM_IndicatorWidth:{
        return 16;
    }
    case PM_IndicatorHeight:{
        return 16;
    }
    case PM_SubMenuOverlap:return -2;
    case PM_ButtonMargin:return  9;
    case PM_DefaultFrameWidth:
        if(const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(option))
        {
            return 4;
        }
        return 2;
    case PM_TabBarTabVSpace:return 20;
    case PM_TabBarTabHSpace:return 40;
    case PM_HeaderMargin:return 9;
    case PM_MenuBarItemSpacing:return 16;
    case PM_MenuBarVMargin:return 4;
    case PM_ProgressBarChunkWidth: return 0;
    case PM_ToolBarItemSpacing:return 4;
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
    default:
        break;
    }
    return QFusionStyle::pixelMetric(metric, option, widget);
}

QRect Qt5UKUIStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        auto rect = QFusionStyle::subControlRect(control, option, subControl, widget);
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
        switch( subControl )
        {
        case SC_SliderHandle:
        {
            QRect handleRect( QFusionStyle::subControlRect( CC_Slider, option, subControl, widget ) );
            handleRect = centerRect( handleRect, PM_SliderThickness+9, PM_SliderControlThickness+8);
            return handleRect;
        }
        default:return QFusionStyle::subControlRect(control, option, subControl, widget);
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
            const int margin = combobox->frame ? 3 : 0;
            const int bmarg = combobox->frame ? 2 : 0;
            int ArrowWidth = option->rect.height();
            if(ArrowWidth > 24)
                ArrowWidth = 24;
            if(ArrowWidth > combobox->rect.width() * 4 / 9)
                ArrowWidth = option->rect.width() * 4 / 9;
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
                    rect.setRect(rect.left(),rect.top(),rect.width() - ArrowWidth - gap,rect.height());
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

    default:
        break;
    }
    return QFusionStyle::subControlRect(control, option, subControl, widget);
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
            if (auto button = qobject_cast<QAbstractButton *>(widget)) {
                opt.state.setFlag(QStyle::State_Sunken, button->isDown());
            }
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
