#include "ukui-style.h"
#include <ukui-style-common.h>
#include <ukui-painter-helper.h>

#include <QMenu>
#include <QLineEdit>
#include <QPushButton>



UKUIStyle::UKUIStyle(StyleType type) : QFusionStyle(), m_type(type)
{
    if (auto settings = UKUIStyleSettings::globalInstance()) {
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "highlightColor") {
                QString highlightMode = settings->get("highlightColor").toString();
                QPalette palette = QApplication::palette();
                setHighLightPalette(palette, highlightMode);
                palette.setColor(QPalette::Disabled, QPalette::Highlight, Qt::transparent);
                qApp->setPalette(palette);
                qApp->paletteChanged(palette);
            }
        });
    }

    m_button_animation_helper = new ButtonAnimationHelper(this);
}



const QStringList UKUIStyle::useLightPalette() const
{
    QStringList l;
    l<<"kylin-assistant";
    l<<"kybackup";
    l<<"biometric-manager";
    return l;
}

const QStringList UKUIStyle::useDefaultPalette() const
{
    QStringList l;
    l<<"ukui-menu";
    l<<"ukui-panel";
    l<<"ukui-sidebar";
    l<<"ukui-volume-control-applet-qt";
    l<<"ukui-power-manager-tray";
    l<<"kylin-nm";
    l<<"ukui-flash-disk";
    l<<"indicator-china-weather";
    return l;
}



void UKUIStyle::setHighLightPalette(QPalette &palette, QString mode) const
{
    if (mode == "daybreak-blue") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(64, 169, 251));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(55, 144, 250));
    } else if (mode == "jam-purple") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 46, 209));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(146, 84, 222));
    } else if (mode == "magenta") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(235, 48, 150));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(245, 89, 171));
    } else if (mode == "sun-red") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(243, 34, 45));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(244, 78, 80));
    } else if (mode == "sunset-orange") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(246, 140, 39));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(247, 169, 64));
    } else if (mode == "dust-gold") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(249, 197, 61));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(250, 214, 102));
    } else if (mode == "polar-green") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(82, 196, 41));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(115, 209, 61));
    } else if (mode == "geek-grey") {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 46, 209));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(140, 140, 140));
    } else {
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(64, 169, 251));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(55, 144, 250));
    }
}



QPalette UKUIStyle::standardPalette() const
{
    QPalette palette = QFusionStyle::standardPalette();

    QColor  window_bg(231,231,231),
            window_no_bg(233,233,233),
            base_bg(255,255,255),
            base_no_bg(248, 248, 248),
            font_bg(0,0,0),
            font_br_bg(255,255,255),
            font_di_bg(191,191,191),
            button_active(245, 245, 245),
            button_inactive(235, 235, 235),
            button_disable(233, 233, 233),
            tip_bg(248,248,248),
            tip_font(22,22,22),
            alternateBase_active(92, 92, 92),
            alternateBase_inactive(92, 92, 92),
            alternateBase_disable(92, 92, 92),
            buttontext_active(38, 38, 38),
            buttontext_inactive(38, 38, 38),
            buttontext_disable(179, 179, 179),
            highlight_active(64, 169, 251),
            highlight_inactive(55, 144, 250),
            highlight_disable(233, 233, 233),
            highlighttext_active(255, 255, 255),
            highlightext_inactive(255, 255, 255),
            highlightext_disable(255, 255, 255);

    if (!useLightPalette().contains(qAppName()) && (m_type == UKUIStyle::udark || ((m_type == UKUIStyle::udefault) && useDefaultPalette().contains(qAppName())))) {
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
        button_active.setRgb(77, 77, 77);
        button_inactive.setRgb(51, 51, 51);
        button_disable.setRgb(48,48,48);
        tip_bg.setRgb(61,61,65);
        tip_font.setRgb(232,232,232);
        buttontext_active.setRgb(217, 217, 217);
        buttontext_inactive.setRgb(217, 217, 217);
        buttontext_disable.setRgb(77, 77, 77);
        highlight_active.setRgb(64, 169, 251);
        highlight_inactive.setRgb(55, 144, 250);
        highlight_disable.setRgb(48, 48, 48);
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

    palette.setBrush(QPalette::PlaceholderText,font_di_bg);

    palette.setBrush(QPalette::ToolTipBase,tip_bg);
    palette.setBrush(QPalette::ToolTipText,tip_font);

    palette.setBrush(QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Active,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Inactive,QPalette::BrightText,font_br_bg);
    palette.setBrush(QPalette::Disabled,QPalette::BrightText,font_di_bg);

    palette.setBrush(QPalette::Active, QPalette::Button, button_active);
    palette.setBrush(QPalette::Inactive, QPalette::Button, button_inactive);
    palette.setBrush(QPalette::Disabled, QPalette::Button, button_disable);

    palette.setBrush(QPalette::Active, QPalette::ButtonText, buttontext_active);
    palette.setBrush(QPalette::Inactive, QPalette::ButtonText, buttontext_inactive);
    palette.setBrush(QPalette::Disabled, QPalette::ButtonText, buttontext_disable);

    palette.setBrush(QPalette::Active, QPalette::AlternateBase, alternateBase_active);
    palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, alternateBase_inactive);
    palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, alternateBase_disable);

    palette.setBrush(QPalette::Active, QPalette::Highlight, highlight_active);
    palette.setBrush(QPalette::Inactive, QPalette::Highlight, highlight_inactive);
    palette.setBrush(QPalette::Disabled, QPalette::Highlight, highlight_disable);

    palette.setBrush(QPalette::Active, QPalette::HighlightedText, highlighttext_active);
    palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, highlightext_inactive);
    palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, highlightext_disable);

    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString mode = settings->get("highlightColor").toString();
        setHighLightPalette(palette, mode);
    }

    return palette;
}



int UKUIStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_UnderlineShortcut:
        return true;

    default:
        break;
    }

    return QFusionStyle::styleHint(hint, option, widget, returnData);
}



void UKUIStyle::polish(QPalette &palette)
{
    palette = standardPalette();
    return QFusionStyle::polish(palette);
}



void UKUIStyle::polish(QApplication *application)
{
    return QFusionStyle::polish(application);
}



void UKUIStyle::polish(QWidget *widget)
{
    QFusionStyle::polish(widget);
    if (qobject_cast<QPushButton*>(widget)) {
        m_button_animation_helper->registerWidget(widget);
    }
}



void UKUIStyle::unpolish(QWidget *widget)
{
    if (qobject_cast<QPushButton*>(widget)) {
        m_button_animation_helper->unregisterWidget(widget);
    }
    QFusionStyle::unpolish(widget);
}



void UKUIStyle::unpolish(QApplication *application)
{
    return QFusionStyle::unpolish(application);
}



void UKUIStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_PushButton: {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
            return;
        }
    }

    case CE_PushButtonBevel: {
        if (qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
            return;
        }
    }

    case CE_PushButtonLabel: {
        if (UKUIPainterHelper::drawPushButtonLable(option, painter, proxy(), widget))
            return;
        else
            break;
    }

    default:
        break;
    }

    return QFusionStyle::drawControl(element, option, painter, widget);
}




void UKUIStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_PanelButtonCommand: {
        if (auto animtor = m_button_animation_helper->animator(widget))
            if (UKUIPainterHelper::drawPushButtonBevel(option, painter, animtor, widget))
            return;
        break;
    }

    default:
        break;
    }

    return QFusionStyle::drawPrimitive(element, option, painter, widget);
}



int UKUIStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option,  const QWidget *widget) const
{
    switch (metric) {
    case QStyle::PM_DefaultFrameWidth: {
        if (qobject_cast<const QMenu*>(widget))
            return UKUIStyleCommon::Menu_FrameWidth;
        if (qobject_cast<const QLineEdit*>(widget))
            return UKUIStyleCommon::LineEdit_FrameWidth;
        return UKUIStyleCommon::Frame_FrameWidth;
    }

    case PM_ButtonMargin:
        return UKUIStyleCommon::Button_MarginWidth;

    case PM_ButtonDefaultIndicator:
        return UKUIStyleCommon::Button_DefaultIndicator;

    default:
        break;
    }

    return QFusionStyle::pixelMetric(metric, option, widget);
}



QSize UKUIStyle::sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    switch (ct) {
    case CT_PushButton: {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = size.width(), h = size.height();
            int bm = proxy()->pixelMetric(PM_ButtonMargin, button, widget);
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, button, widget) * 2;
            w += bm + fw;
            h += bm + fw;
            w += (UKUIStyleCommon::Button_IconTextDistance - 4);
            if (button->features & QStyleOptionButton::AutoDefaultButton) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, button, widget) * 2;
                w += dbw;
                h += dbw;
            }

            //set minsize
            if (!button->text.isEmpty() && w < UKUIStyleCommon::Button_MinWideth)
                w = UKUIStyleCommon::Button_MinWideth;
            if (h < UKUIStyleCommon::Button_MinHight)
                h = UKUIStyleCommon::Button_MinHight;

            if (!button->icon.isNull() && button->iconSize.height() > 16)
                h -+ 2;
            return QSize(w, h);
        }
    }

    default:
        break;
    }

    return QFusionStyle::sizeFromContents(ct, option, size, widget);
}
