#include "ukui-style.h"

UKUIStyle::UKUIStyle(StyleType type) : QFusionStyle(), m_type(type)
{
    if (auto settings = UKUIStyleSettings::globalInstance()) {
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "highlightColor") {
                QString highlightMode = settings->get("highlightColor").toString();
                QPalette palette = QApplication::palette();
                if (highlightMode == "daybreakBlue") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(55, 144, 250));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(64, 169, 251));
                } else if (highlightMode == "jamPurple") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 46, 209));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(146, 84, 222));
                } else if (highlightMode == "magenta") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(235, 48, 150));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(245, 89, 171));
                } else if (highlightMode == "sunRed") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(243, 34, 45));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(244, 78, 80));
                } else if (highlightMode == "sunsetOrange") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(246, 140, 39));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(247, 169, 64));
                } else if (highlightMode == "dustGold") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(249, 197, 61));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(250, 214, 102));
                } else if (highlightMode == "polarGreen") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(82, 196, 41));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(115, 209, 61));
                } else if (highlightMode == "geekGrey") {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 46, 209));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(140, 140, 140));
                } else {
                    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(55, 144, 250));
                    palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(64, 169, 251));
                }
                palette.setColor(QPalette::Disabled, QPalette::Highlight, Qt::transparent);
                qApp->setPalette(palette);
                qApp->paletteChanged(palette);
            }
        });
    }
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
            button_bg(217,217,217),
            button_ac_bg(107,142,235),
            button_di_bg(233,233,233),
            highlight_bg(61,107,229),
            tip_bg(248,248,248),
            tip_font(22,22,22),
            alternateBase(248,248,248);

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
