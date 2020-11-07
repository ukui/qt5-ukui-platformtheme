#include "ukui-painter-helper.h"
#include "ukui-style-common.h"



bool UKUIPainterHelper::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, AnimatorIface *animator, const QWidget *widget)
{
    if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
        const QRect& rect(option->rect);
        const QPalette& palette(option->palette);
        const bool enable = button->state & QStyle::State_Enabled;
        const bool mouseover = button->state & QStyle::State_MouseOver;
        const bool sunken = button->state & QStyle::State_Sunken;
        const bool on = button->state & QStyle::State_On;
        const bool focus = (button->state & QStyle::State_HasFocus) && (widget && widget->focusPolicy());

        if (button->features & QStyleOptionButton::Flat) {
            return true;
        }
        if (!enable) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(palette.color(QPalette::Disabled, QPalette::Button));
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();
            return true;
        }

        if (animator == nullptr) {
            return false;
        }

        QBrush normal_brush = palette.brush(QPalette::Inactive, QPalette::Button);
        QBrush mouseover_brush = palette.brush(QPalette::Active, QPalette::Button);
        QBrush sunken_brush(UKUIColorHelper::get_SunKen_Button_Color());
        QBrush disable_brush = palette.brush(QPalette::Disabled, QPalette::Button);
        if (button->features & (QStyleOptionButton::DefaultButton | QStyleOptionButton::AutoDefaultButton)) {
            normal_brush = palette.brush(QPalette::Inactive, QPalette::Highlight);
            mouseover_brush = palette.brush(QPalette::Active, QPalette::Highlight);
            sunken_brush = QBrush(UKUIColorHelper::get_SunKen_HighLight_Color());
            disable_brush = palette.brush(QPalette::Disabled, QPalette::Highlight);
        }

        if (!(button->state & QStyle::State_AutoRaise)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(normal_brush);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(option->rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();
        }

        if (sunken  || on || animator->isRunning("SunKen") || animator->value("SunKen") == 1.0) {
            double opacity = animator->value("SunKen").toDouble();
            if (sunken | on) {
                if (opacity == 0.0) {
                    animator->setAnimatorDirectionForward("SunKen", true);
                    animator->startAnimator("SunKen");
                }
            } else if (opacity == 1.0) {
                animator->setAnimatorDirectionForward("SunKen", false);
                animator->startAnimator("SunKen");
            }

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(mouseover_brush);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(sunken_brush);
            painter->setOpacity(animator->value("SunKen").toDouble());
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();
            return true;
        }

        if (mouseover || animator->isRunning("MouseOver") || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
            double opacity = animator->value("MouseOver").toDouble();
            if (mouseover) {
                animator->setAnimatorDirectionForward("MouseOver", true);
                if (opacity == 0.0)
                    animator->startAnimator("MouseOver");
            } else {
                animator->setAnimatorDirectionForward("MouseOver", false);
                if (opacity == 1.0)
                    animator->startAnimator("MouseOver");
            }

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(mouseover_brush);
            painter->setOpacity(animator->value("MouseOver").toDouble());
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();
            return true;
        }

        if (focus) {
            painter->save();
            painter->setPen(QPen(UKUIColorHelper::get_SunKen_HighLight_Color(), 2));
            painter->setClipRect(rect);
            painter->setBrush(normal_brush);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawRoundedRect(rect, UKUIStyleCommon::Frame_xRadius, UKUIStyleCommon::Frame_yRadius);
            painter->restore();
            return true;
        }
        return true;
    }
    return false;
}



bool UKUIPainterHelper::drawPushButtonLable(const QStyleOption *option, QPainter *painter, const QStyle *style, const QWidget *widget)
{
    if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
        const QRect& rect(button->rect);
        const QPalette& palette(button->palette);
        const bool enable = button->state & QStyle::State_Enabled;
        const bool mouseOver = button->state & QStyle::State_MouseOver;
        const bool sunken = button->state & QStyle::State_Sunken;
        const bool on = button->state & QStyle::State_On;
        const bool focus = (button->state & QStyle::State_HasFocus) && (widget && widget->focusPolicy());

        int bm = style->pixelMetric(QStyle::PM_ButtonMargin, button, widget);
        int bfw = style->pixelMetric(QStyle::PM_DefaultFrameWidth, button, widget) * 2;


        QRect drawRect = rect_ChangeSize(rect, -bm - bfw, -bm - bfw);
        //        if (button->features & QStyleOptionButton::HasMenu)
        //        {
        //            QStyleOptionButton arrow = *button;
        //            int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
        //            if (button->direction == Qt::RightToLeft)
        //            {
        //                arrow.rect.setRect(drawRect.left(),drawRect.top(),indicatorSize,drawRect.height());
        //                drawRect = drawRect.adjusted(indicatorSize, 0, 0, 0);
        //            }
        //            else
        //            {
        //                drawRect = drawRect.adjusted(0, 0, -indicatorSize, 0);
        //                arrow.rect.setRect(drawRect.right(),drawRect.top(),indicatorSize,drawRect.height());
        //            }
        //            style->drawPrimitive(QStyle::PE_IndicatorArrowDown,&arrow,painter,widget);
        //        }

        if (!button->icon.isNull()) {
            QIcon::Mode mode = button->state & enable ? QIcon::Normal : QIcon::Disabled;
            if (mode == QIcon::Normal && focus)
                mode = QIcon::Active;
            QIcon::State state = QIcon::Off;
            if (on)
                state = QIcon::On;
            QPixmap pixmap = button->icon.pixmap(button->iconSize, mode, state);
            if (button->text.isEmpty()) {
                style->drawItemPixmap(painter, drawRect, Qt::AlignCenter, pixmap);
                return true;
            }

            QRect textRect = button->fontMetrics.boundingRect(button->text);
            QRect iconRect = drawRect;
            iconRect.setWidth(button->iconSize.width() + UKUIStyleCommon::Button_IconTextDistance + textRect.width());
            iconRect.moveCenter(drawRect.center());
            iconRect.setWidth(button->iconSize.width());
            textRect.setRect(iconRect.right() + UKUIStyleCommon::Button_IconTextDistance, iconRect.top(), textRect.width(), textRect.height());
            iconRect = QStyle::visualRect(button->direction, drawRect, iconRect);
            textRect = QStyle::visualRect(button->direction, drawRect, textRect);

            style->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            drawRect = textRect;
        }

        uint tf = Qt::AlignCenter;
        if (style->styleHint(QStyle::SH_UnderlineShortcut, button, widget))
            tf |= Qt::TextShowMnemonic;
        else
            tf |= Qt::TextHideMnemonic;

        QColor text_color = palette.color(QPalette::Active, QPalette::ButtonText);
        if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
            text_color = palette.color(QPalette::Active, QPalette::HighlightedText);
        }
        if (button->features & QStyleOptionButton::Flat) {
            if (sunken | on)
                text_color = UKUIColorHelper::get_SunKen_HighLight_Color();
            else if (mouseOver)
                text_color = palette.color(QPalette::Active, QPalette::Highlight);
            else
                text_color = palette.color(QPalette::Active, QPalette::ButtonText);
        }
        if (!enable)
            text_color = palette.color(QPalette::Disabled, QPalette::ButtonText);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(text_color);
        painter->setBrush(Qt::NoBrush);
        painter->drawText(drawRect, tf, button->text);
        painter->restore();
        return true;
    }
    return false;
}
