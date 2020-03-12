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

#include "tab-widget-animation-helper.h"
#include "scrollbar-animation-helper.h"

#include "animator-iface.h"

#include <QIcon>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QtPlatformHeaders/QXcbWindowFunctions>

#include <QEvent>
#include <QDebug>

Qt5UKUIStyle::Qt5UKUIStyle(bool dark) : QFusionStyle ()
{
    m_use_dark_palette = dark;
    m_tab_animation_helper = new TabWidgetAnimationHelper(this);
    m_scrollbar_animation_helper = new ScrollBarAnimationHelper(this);
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
    //ukui-white
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
            tip_bg(248,248,248);

    if (m_use_dark_palette) {
        //ukui-black
        window_bg.setRgb(36,36,38);
        window_no_bg.setRgb(48,46,50);
        base_bg.setRgb(0,0,0);
        base_no_bg.setRgb(28,28,30);
        font_bg.setRgb(255,255,255);
        font_br_bg.setRgb(255,255,255);
        font_di_bg.setRgb(28,28,30);
        button_bg.setRgb(44,44,46);
        button_ac_bg.setRgb(97,97,102);
        button_di_bg.setRgb(52,52,56);
        highlight_bg.setRgb(61,107,229);
        tip_bg.setRgb(26,26,26);
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
    palette.setBrush(QPalette::ToolTipText,font_bg);

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

    palette.setBrush(QPalette::AlternateBase,button_bg);
    palette.setBrush(QPalette::Inactive,QPalette::AlternateBase,button_ac_bg);
    palette.setBrush(QPalette::Disabled,QPalette::AlternateBase,button_di_bg);

    return palette;
}

void Qt5UKUIStyle::polish(QWidget *widget)
{
    QFusionStyle::polish(widget);

    if (widget->inherits("QMenu")) {
        widget->setAttribute(Qt::WA_TranslucentBackground);
        //QRegion mask = getRoundedRectRegion(widget->rect(), 10, 10);

        //widget->setMask(mask);
        //do not polish widget with proxy style.
        return;
        //qDebug()<<mask<<"menu mask"<<widget->mask();
    }

    if (widget->inherits("QTabWidget")) {
        //FIXME: unpolish, extensiable.
        m_tab_animation_helper->registerWidget(widget);
    }

    if (widget->inherits("QScrollBar")) {
        widget->setAttribute(Qt::WA_Hover);
        m_scrollbar_animation_helper->registerWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    widget->installEventFilter(this);
}

void Qt5UKUIStyle::unpolish(QWidget *widget)
{
    widget->removeEventFilter(this);

    if (widget->inherits("QMenu")) {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        //widget->setMask(QRegion());
        return;
    }

    if (widget->inherits("QTabWidget")) {
        m_tab_animation_helper->unregisterWidget(widget);
    }

    if (widget->inherits("QScrollBar")) {
        widget->setAttribute(Qt::WA_Hover, false);
        m_scrollbar_animation_helper->unregisterWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
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
        /*!
          \bug
          a "disabled" menu paint and blur in error, i have no idea about that.
          */
        if (widget->isEnabled()) {
            return drawMenuPrimitive(option, painter, widget);
        }
        return QFusionStyle::drawPrimitive(element, option, painter, widget);
    }
    case PE_FrameFocusRect: {
        if (qobject_cast<const QAbstractItemView *>(widget))
            return;
        break;
    }
    case PE_PanelItemViewRow: {
        return;
    }
    case PE_PanelItemViewItem: {
        /*!
         * \todo
         * deal with custom/altenative background items.
         */
        bool isHover = (option->state & State_MouseOver) && (option->state & ~State_Selected);
        bool isSelected = option->state & State_Selected;
        bool enable = option->state & State_Enabled;
        QColor color = option->palette.color(enable? QPalette::Active: QPalette::Disabled,
                                             QPalette::Highlight);
        color.setAlpha(0);
        if (isHover) {
            color.setAlpha(127);
        }
        if (isSelected) {
            color.setAlpha(255);
        }
        painter->fillRect(option->rect, color);
        return;
    }

    case PE_PanelButtonCommand://UKUI PushButton style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        /*!
       * \todo
       * It is selected by default and not done first,because it conflicts with the normal button.
       */
        //        if(option->state & State_HasFocus){
        //            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Button));
        //            painter->setBrush(option->palette.color(QPalette::Highlight));
        //        }
        //        else {
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Button));
        //        }

        if (option->state & State_MouseOver) {
            if (option->state & State_Sunken) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Highlight));
            } else {
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Highlight));
            }
        }
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();

        /*!
       * \todo
       * Judge whether it is OK or other buttons
       */

        return;
    }

    case PE_PanelTipLabel://UKUI Tip  style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::ToolTipText));
        painter->setBrush(option->palette.color(QPalette::ToolTipBase));
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();
        return;
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
                painter->drawRoundedRect(option->rect.adjusted(+0,+0,-4,+0),0,0);
                painter->drawRoundedRect(option->rect.adjusted(+1,+0,+0,+0),4,4);
            }
        }
        painter->restore();
        return;
    }

    case PE_PanelButtonTool://UKUI ToolBar  item style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::NoBrush);

        if (widget->isEnabled()) {
            if (option->state & State_MouseOver) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
            }
            if (option->state & State_Sunken) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
            }
        }

        /*To determine whether the pop-up menu is in progress,
        it is a combination button, and the shape needs to be changed*/
        const QToolButton* toolButton = qobject_cast<const QToolButton*>( widget );
        const bool hasPopupMenu( toolButton && toolButton->popupMode() == QToolButton::MenuButtonPopup );
        if( hasPopupMenu )
        {
            painter->drawRoundedRect(option->rect,4,4);
            painter->drawRoundedRect(option->rect.adjusted(+4,+0,+0,+0),0,0);
        }
        else {
            painter->drawRoundedRect(option->rect,4,4);
        }
        painter->restore();
        return;
    }

    case PE_FrameTabWidget://UKUI TabBar item style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setBrush(option->palette.color(QPalette::Base));
        painter->setPen(option->palette.color(QPalette::Base));
        painter->drawRect(option->rect.adjusted(+3,+2,+0,-10));
        painter->drawRoundedRect(option->rect.adjusted(+3,+10,+0,+0),5,5);
        painter->restore();
        return;
    }

    case PE_FrameTabBarBase://UKUI TabBar style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::Button));
        painter->setBrush(option->palette.color(QPalette::Button));
        painter->drawRoundedRect(option->rect,0,0);
        painter->restore();
        return;
    }

    case PE_FrameGroupBox://UKUI GroupBox style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::Base));
        painter->setBrush(option->palette.color(QPalette::Base));
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();
        return;
    }

    case PE_PanelLineEdit://UKUI Text edit style
    {
        // Conflict with qspinbox and so on, The widget text cannot use this style
        if (widget) {
            if (widget->parentWidget()) {
                if (widget->parentWidget()->inherits("QDoubleSpinBox")|widget->parentWidget()->inherits("QSpinBox")|widget->parentWidget()->inherits("QComboBox")) {
                    return;
                }
            }
        }
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Base));
        if (widget->isEnabled()) {
            if (option->state &State_MouseOver) {
                painter->setBrush(option->palette.color(QPalette::Mid));
            }
            if(option->state &State_HasFocus) {
                painter->setPen(option->palette.color(QPalette::Highlight));
                painter->setBrush(option->palette.color(QPalette::Base));
            }
        }
        painter->drawRoundedRect(option->rect,4,4);
        painter->restore();
        return;
    }

    case PE_IndicatorArrowDown:case PE_IndicatorArrowUp:case PE_IndicatorArrowLeft:case PE_IndicatorArrowRight:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setBrush(Qt::NoBrush);
        if(option->state & State_Enabled){
            painter->setPen(QPen(option->palette.foreground().color(), 1.1));
            if (option->state & State_MouseOver) {
                painter->setPen(QPen(option->palette.color(QPalette::Highlight), 1.1));

                //If the type is toolbar button, change the color when hover
                const QToolButton* toolButton = qobject_cast<const QToolButton*>( widget );
                const bool hasPopupMenu( toolButton && toolButton->popupMode() == QToolButton::MenuButtonPopup );
                if( hasPopupMenu )
                {
                    if (option->state & State_MouseOver|State_Sunken) {
                        painter->setPen(QPen(option->palette.color(QPalette::Light), 1.1));
                    }
                }
            }
        }
        else {
            painter->setPen(QPen(option->palette.color(QPalette::Text), 1.1));
        }

        QPolygon points(4);
        int x = option->rect.x();
        int y = option->rect.y();
        //If the height is too high, the arrow will be very ugly. If the height is too small, the arrow will not be painted
        // int w = option->rect.width() / 3;
        // int h =  option->rect.height() / 4;
        int w = 8;
        int h =  4;
        x += (option->rect.width() - w) / 2;
        y += (option->rect.height() - h) / 2;

        //When the arrow is too small, you can not draw
        if(option->rect.width() - w < 1 || option->rect.height() - h < 1){
            return;
        }
        else if (option->rect.width() - w <= 2 || option->rect.height() - h <= 2){
            w = 5;
            h =  3;
        }

        if (element == PE_IndicatorArrowDown) {
            points[0] = QPoint(x, y);
            points[1] = QPoint(x + w / 2, y + h);
            points[2] = QPoint(x + w / 2, y + h);
            points[3] = QPoint(x + w, y);
        }

        //When left and right, "W" and "H" are interchanged so that the arrow does not deform
        else if (element == PE_IndicatorArrowUp) {
            points[0] = QPoint(x, y + h);
            points[1] = QPoint(x + w / 2, y);
            points[2] = QPoint(x + w / 2, y);
            points[3] = QPoint(x + w, y + h);
        }
        else if (element == PE_IndicatorArrowLeft) {
            points[0] = QPoint(x + h , y);
            points[1] = QPoint(x, y+w/2);
            points[2] = QPoint(x, y+w/2);
            points[3] = QPoint(x + w , y + w);
        }
        else if (element == PE_IndicatorArrowRight) {
            points[0] = QPoint(x , y);
            points[1] = QPoint(x+h, y+w/2);
            points[2] = QPoint(x+h , y+w/2);
            points[3] = QPoint(x, y+w);
        }
        painter->drawLine(points[0],  points[1] );
        painter->drawLine(points[2],  points[3] );
        painter->restore();
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
        painter->drawRect(grooveRect);
        painter->restore();

        return QCommonStyle::drawComplexControl(control, option, painter, widget);
    }

    case CC_ComboBox:
    {
        QRect rect=subControlRect(CC_ComboBox,option,SC_ComboBoxFrame);
        //adjusted(+1,+1,-1,-1)
        painter->save();
        painter->setPen(option->palette.color(QPalette::Button));
        painter->setBrush(option->palette.color(QPalette::Button));
        painter->setRenderHint(QPainter::Antialiasing,true);
        if (widget->isEnabled()) {
            if (option->state & State_MouseOver) {
                if (option->state & State_Sunken) {
                    painter->setPen(option->palette.color(QPalette::Highlight));

                } else {
                    painter->setPen(option->palette.color(QPalette::Highlight));
                }
            }
            if (option->state & State_On) {
                painter->setPen(option->palette.color(QPalette::Highlight));
            }
        }
        painter->drawRoundedRect(rect,4,4);
        painter->restore();
        drawComBoxIndicator(SC_ComboBoxArrow,option,painter);
        return;
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
                pen.setColor(option->palette.color(QPalette::Highlight));
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
            return;}

    default:        return QFusionStyle::drawComplexControl(control, option, painter, widget);
    }
}

void Qt5UKUIStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
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
                sliderWidth = qMin(animator->value("groove_width").toInt() + 2, option->rect.height());
            } else {
                sliderWidth = qMin(animator->value("groove_width").toInt() + 2, option->rect.width());
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

            if (animator->isRunning("slider_opacity")) {
                const_cast<QWidget *>(widget)->update();
            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::transparent);
            painter->setBrush(option->palette.windowText());
            painter->setOpacity(animator->value("slider_opacity").toDouble());
            auto sliderRect = option->rect;
            if (is_horizontal) {
                sliderRect.setY(sliderRect.height() - sliderWidth);
            } else {
                sliderRect.setX(sliderRect.width() - sliderWidth);
            }
            if (sliderWidth > 3) {
                if (is_horizontal) {
                    sliderRect.adjust(0, 1, 0, -1);
                } else {
                    sliderRect.adjust(1, 0, -1, 0);
                }
            } else {
                if (is_horizontal) {
                    sliderRect.adjust(0, -2, 0, -2);
                } else {
                    sliderRect.adjust(-2, 0, -2, 0);
                }
            }
            painter->drawRoundedRect(sliderRect, 6, 6);
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

    case CE_PushButtonLabel:
    {
        const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option);
        QRect textRect = button->rect;
        //这是是否要绘制"&P" as P（带下划线）
        uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
        if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
            tf |= Qt::TextHideMnemonic;

        if (!button->icon.isNull()) {
            //Center both icon and text
            QRect iconRect;
            QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
            if (mode == QIcon::Normal && button->state & State_HasFocus)
                mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (button->state & State_On)
                state = QIcon::On;

            QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);
            int labelWidth = pixmap.width();
            int labelHeight = pixmap.height();
            int iconSpacing = 4;//4 is currently hardcoded in QPushButton::sizeHint()
            int textWidth =button->fontMetrics.boundingRect(option->rect, int(tf), button->text).width();
            if (!button->text.isEmpty())
                labelWidth += (textWidth + iconSpacing);

            iconRect = QRect(textRect.x() + (textRect.width() - labelWidth) / 2,
                             textRect.y() + (textRect.height() - labelHeight) / 2,
                             pixmap.width(), pixmap.height());

            iconRect = visualRect(button->direction, textRect, iconRect);

            tf |= Qt::AlignLeft; //left align, we adjust the text-rect instead

            if (button->direction == Qt::RightToLeft)
                textRect.setRight(iconRect.left() - iconSpacing);
            else
                textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

            if (button->state & (State_On | State_Sunken))
                iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                   proxy()->pixelMetric(PM_ButtonShiftVertical, option, widget));
            painter->drawPixmap(iconRect, pixmap);
        } else {
            tf |= Qt::AlignHCenter;
        }
        if (button->state & (State_On | State_Sunken))
            textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                               proxy()->pixelMetric(PM_ButtonShiftVertical, option, widget));

        if (button->features & QStyleOptionButton::HasMenu) {
            int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
            if (button->direction == Qt::LeftToRight)
                textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
            else
                textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
        }
        //You can also write static colors directly
        //proxy()->drawItemText(painter, textRect, tf, button->palette, (button->state & State_Enabled),button->text, QPalette::HighlightedText);

        //The following are text dynamic colors
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        if(option->state & State_HasFocus){
            painter->setPen(option->palette.color(QPalette::HighlightedText));
        }
        else {
            painter->setPen(option->palette.color(QPalette::ButtonText));
        }
        if (option->state & State_MouseOver) {
            if (option->state & State_Sunken) {
                painter->setPen(option->palette.color(QPalette::HighlightedText));
            } else {
                painter->setPen(option->palette.color(QPalette::HighlightedText));
            }
        }
        //painter->drawText(option->rect,pushbutton->text, QTextOption(Qt::AlignCenter));
        proxy()->drawItemText(painter, textRect, int(tf), button->palette, (button->state & State_Enabled),button->text);
        painter->restore();
        return;
    }

    case CE_TabBarTabShape://UKUI TabBar item style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::Button));
        painter->setBrush(option->palette.color(QPalette::Button));
        if (option->state & State_Selected) {
            if (option->state & State_Sunken) {
                painter->setPen(option->palette.color(QPalette::Base));
                painter->setBrush(option->palette.color(QPalette::Base));
            } else {
                painter->setPen(option->palette.color(QPalette::Base));
                painter->setBrush(option->palette.color(QPalette::Base));
            }
        }
        painter->drawRoundedRect(option->rect.adjusted(+3,+0,+0,-option->rect.height()/2),4,4);
        painter->drawRect(option->rect.adjusted(+3,+option->rect.height()/2-4,+0,+0));
        painter->restore();
        return;
    }

    case CE_ComboBoxLabel:
    {
        auto comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox*>(option);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::ButtonText));
        if (option->state & State_Selected) {
            if (option->state & State_Sunken) {
                painter->setPen(option->palette.color(QPalette::ButtonText));
            } else {
                painter->setPen(option->palette.color(QPalette::ButtonText));
            }
        }
        painter->drawText(option->rect.adjusted(+4,+0,+0,+0), comboBoxOption->currentText, QTextOption(Qt::AlignVCenter));
        painter->restore();
        return;
    }


    case CE_CheckBox:
    {
        auto checkbutton = qstyleoption_cast<const QStyleOptionButton*>(option);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::Text));
        painter->drawText(option->rect.adjusted(+20,+0,0,0),checkbutton->text);
        painter->restore();

        painter->save();
        //  if (option->state & State_None){//Non optional status
        painter->save();
        painter->setBrush(option->palette.color(QPalette::Disabled,QPalette::Button));
        painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
        painter->drawRoundedRect(option->rect.x(),option->rect.y()+1,option->rect.x()+16,option->rect.x()+16,2,2);
        painter->restore();
        painter->save();
        painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Text));
        painter->drawText(option->rect.adjusted(+20,+0,0,0),checkbutton->text);
        painter->restore();
        //} else
        if (option->state & State_Off) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Button));
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Button));
            if (option->state & State_Sunken) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }else if (option->state & State_MouseOver){
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }
            painter->drawRoundedRect(option->rect.x(),option->rect.y()+1,option->rect.x()+16,option->rect.x()+16,2,2);
            painter->restore();
        } else if (option->state & State_On) {
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
            painter->drawRoundedRect(option->rect.x(),option->rect.y()+1,option->rect.x()+16,option->rect.x()+16,2,2);
            painter->restore();

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(QPen(option->palette.color(QPalette::HighlightedText),1.3));
            painter->drawLine(int(option->rect.x()+4.5),option->rect.y()+8,int(option->rect.x())+8,int(option->rect.y())+13);
            painter->drawLine(int(option->rect.x())+9,option->rect.y()+12,int(option->rect.x()+13.5),option->rect.y()+6);
            painter->restore();
        }
        painter->restore();
        return;
    }

    case CE_RadioButton:{
        auto radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(option->palette.color(QPalette::Text));
        painter->drawText(option->rect.adjusted(+20,+1,0,0),radiobutton->text);
        painter->restore();

        if (option->state & State_None){//Non optional status
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Disabled,QPalette::Button));
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Button));
            painter->drawEllipse(option->rect.x()+1,option->rect.y()+1, 16.0, 16.0);
            painter->restore();
            painter->save();
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Text));
            painter->drawText(option->rect.adjusted(+20,+1,0,0),radiobutton->text);
            painter->restore();
        } else if (option->state & State_Off) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setBrush(option->palette.color(QPalette::Button));
            painter->setPen(option->palette.color(QPalette::Disabled,QPalette::Button));
            if (option->state & State_Sunken) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }else if (option->state & State_MouseOver){
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->setPen(QPen(option->palette.color(QPalette::Dark), 1));
            }
            painter->drawEllipse(option->rect.x()+1,option->rect.y()+1, 16.0, 16.0);
            painter->restore();
        } else if (option->state & State_On) {
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
            painter->drawEllipse(option->rect.x()+1,option->rect.y()+1, 16.0, 16.0);
            painter->restore();

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing,true);
            painter->setPen(option->palette.color(QPalette::HighlightedText));
            painter->setBrush(option->palette.color(QPalette::HighlightedText));
            painter->drawEllipse(option->rect.x()+6, option->rect.y()+6, 6.0, 6.0);
        }
        painter->restore();
        return;
    }

    default:
        return QFusionStyle::drawControl(element, option, painter, widget);
    }
}

int Qt5UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent: {
        return 12;
    }
    case PM_ScrollView_ScrollBarOverlap: {
        return -12;
    }
    case PM_SliderThickness:{
        return 20;
    }
    case PM_SliderControlThickness:{
        return 20;
    }
    case PM_MenuHMargin:{
        return 5;
    }
    case PM_MenuVMargin:{
        return 5;
    }
    default:
        break;
    }
    return QFusionStyle::pixelMetric(metric, option, widget);
}

QRect Qt5UKUIStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        return scrollBarSubControlRect(control, option, subControl, widget);
    }
    case CC_Slider:
        switch( subControl )
        {
        case SC_SliderHandle:
        {
            QRect handleRect( QFusionStyle::subControlRect( CC_Slider, option, subControl, widget ) );
            handleRect = centerRect( handleRect, PM_SliderThickness+4, PM_SliderControlThickness+3);
            return handleRect;
        }
        default:return QFusionStyle::subControlRect(control, option, subControl, widget);
        }

    default:
        break;
    }
    return QFusionStyle::subControlRect(control, option, subControl, widget);
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
        drawPrimitive(arrow,&arrowOpt,painter);
    painter->restore();
}


QRect  Qt5UKUIStyle::centerRect(const QRect &rect, int width, int height) const
{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }
