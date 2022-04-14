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
 * Authors: Lei Chen <leichen@kylinos.cn>
 *
 */

#include "kabstract-style-parameters.h"

extern QColor mixColor(const QColor &c1, const QColor &c2, qreal bias);

KAbstractStyleParameters::KAbstractStyleParameters(QObject *parent)
{

}


KDefaultStyleParameters::KDefaultStyleParameters(QObject *parent) : KAbstractStyleParameters(parent)
{
    radius = 6;

    ColoseButtonColor = QColor(243, 34, 45);

    Menu_Radius = 8;
    MenuItem_Radius = 6;
}



QPalette KDefaultStyleParameters::setPalette(QPalette &palette, bool isDark)
{

    //ukui-light palette UKUI3.1
    QColor windowText_at(38, 38, 38),
           windowText_iat(0, 0, 0, 255 * 0.55),
           windowText_dis(0, 0, 0, 255 * 0.3),
           button_at(230, 230, 230),
           button_iat(230, 230, 230),
           button_dis(233, 233, 233),
           light_at(255, 255, 255),
           light_iat(255, 255, 255),
           light_dis(242, 242, 242),
           midlight_at(218, 218, 218),
           midlight_iat(218, 218, 218),
           midlight_dis(230, 230, 230),
           dark_at(77, 77, 77),
           dark_iat(77, 77, 77),
           dark_dis(64, 64, 64),
           mid_at(115, 115, 115),
           mid_iat(115, 115, 115),
           mid_dis(102, 102, 102),
           text_at(38, 38, 38),
           text_iat(38, 38, 38),
           text_dis(0, 0, 0, 255 * 0.3),
           brightText_at(0, 0, 0),
           brightText_iat(0, 0, 0),
           brightText_dis(0, 0, 0),
           buttonText_at(38, 38, 38),
           buttonText_iat(38, 38, 38),
           buttonText_dis(179, 179, 179),
           base_at(255, 255, 255),
           base_iat(245, 245, 245),
           base_dis(237, 237, 237),
           window_at(245, 245, 245),
           window_iat(237, 237, 237),
           window_dis(230, 230, 230),
           shadow_at(0, 0, 0, 255 * 0.16),
           shadow_iat(0, 0, 0, 255 * 0.16),
           shadow_dis(0, 0, 0, 255 * 0.21),
//           shadow_at(214, 214, 214),
//           shadow_iat(214, 214, 214),
//           shadow_dis(201, 201, 201),
           highLight_at(55, 144, 250),
           highLight_iat(55, 144, 250),
           highLight_dis(233, 233, 233),
           highLightText_at(255, 255, 255),
           highLightText_iat(255, 255, 255),
           highLightText_dis(179, 179, 179),
           link_at(55, 144, 250),
           link_iat(55, 144, 250),
           link_dis(55, 144, 250),
           linkVisited_at(114, 46, 209),
           linkVisited_iat(114, 46, 209),
           linkVisited_dis(114, 46, 209),
           alternateBase_at(245, 245, 245),
           alternateBase_iat(245, 245, 245),
           alternateBase_dis(245, 245, 245),
           noRale_at(240, 240, 240),
           noRole_iat(240, 240, 240),
           noRole_dis(217, 217, 217),
           toolTipBase_at(255, 255, 255),
           toolTipBase_iat(255, 255, 255),
           toolTipBase_dis(255, 255, 255),
           toolTipText_at(38, 38, 38),
           toolTipText_iat(38, 38, 38),
           toolTipText_dis(38, 38, 38),
           placeholderText_at(0, 0, 0, 255 * 0.35),
           placeholderText_iat(0, 0, 0, 255 * 0.35),
           placeholderText_dis(0, 0, 0, 255 * 0.3);

    //ukui-dark
    if (isDark) {
        windowText_at.setRgb(217, 217, 217);
        windowText_iat.setRgb(255, 255, 255, 255 * 0.55);
        windowText_dis.setRgb(255, 255, 255, 255 * 0.3);
        button_at.setRgb(55, 55, 59);
        button_iat.setRgb(55, 55, 59);
        button_dis.setRgb(46, 46, 48);
        light_at.setRgb(255, 255, 255),
        light_iat.setRgb(255, 255, 255),
        light_dis.setRgb(242, 242, 242),
        midlight_at.setRgb(95, 95, 98);
        midlight_iat.setRgb(95, 95, 98);
        midlight_dis.setRgb(79, 79, 82);
        dark_at.setRgb(38, 38, 38);
        dark_iat.setRgb(38, 38, 38);
        dark_dis.setRgb(26, 26, 26);
        mid_at.setRgb(115, 115, 115);
        mid_iat.setRgb(115, 115, 115);
        mid_dis.setRgb(102, 102, 102);
        text_at.setRgb(217, 217, 217);
        text_iat.setRgb(217, 217, 217);
        text_dis.setRgb(255, 255, 255, 255 * 0.3);
        brightText_at.setRgb(255, 255, 255);
        brightText_iat.setRgb(255, 255, 255);
        brightText_dis.setRgb(255, 255, 255);
        buttonText_at.setRgb(217, 217, 217);
        buttonText_iat.setRgb(217, 217, 217);
        buttonText_dis.setRgb(76, 76, 79);
        base_at.setRgb(18, 18, 18);
        base_iat.setRgb(28, 28, 28);
        base_dis.setRgb(36, 36, 36);
        window_at.setRgb(35, 36, 38);
        window_iat.setRgb(26, 26, 26);
        window_dis.setRgb(18, 18, 18);
        shadow_at.setRgb(0, 0, 0, 255 * 0.16),
        shadow_iat.setRgb(0, 0, 0, 255 * 0.16),
        shadow_dis.setRgb(0, 0, 0, 255 * 0.21),
//        shadow_at.setRgb(214, 214, 214);
//        shadow_iat.setRgb(214, 214, 214);
//        shadow_dis.setRgb(201, 201, 201);
        highLight_at.setRgb(55, 144, 250);
        highLight_iat.setRgb(55, 144, 250);
        highLight_dis.setRgb(46, 46, 46);
        highLightText_at.setRgb(255, 255, 255);
        highLightText_iat.setRgb(255, 255, 255);
        highLightText_dis.setRgb(77, 77, 77);
        link_at.setRgb(55, 144, 250);
        link_iat.setRgb(55, 144, 250);
        link_dis.setRgb(55, 144, 250);
        linkVisited_at.setRgb(114, 46, 209);
        linkVisited_iat.setRgb(114, 46, 209);
        linkVisited_dis.setRgb(114, 46, 209);
        alternateBase_at.setRgb(38, 38, 38);
        alternateBase_iat.setRgb(38, 38, 38);
        alternateBase_dis.setRgb(38, 38, 38);
        noRale_at.setRgb(51, 51, 51);
        noRole_iat.setRgb(51, 51, 51);
        noRole_dis.setRgb(60, 60, 60);
        toolTipBase_at.setRgb(38, 38, 38);
        toolTipBase_iat.setRgb(38, 38, 38);
        toolTipBase_dis.setRgb(38, 38, 38);
        toolTipText_at.setRgb(217, 217, 217);
        toolTipText_iat.setRgb(217, 217, 217);
        toolTipText_dis.setRgb(217, 217, 217);
        placeholderText_at.setRgb(255, 255, 255, 255 * 0.35);
        placeholderText_iat.setRgb(255, 255, 255, 255 * 0.35);
        placeholderText_dis.setRgb(255, 255, 255, 255 * 0.3);
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
    palette.setColor(QPalette::Disabled, QPalette::Highlight, highLight_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::Link, link_at);
    palette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    palette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    palette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif

    return palette;



}



QColor KDefaultStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}




KClassicalStyleParameters::KClassicalStyleParameters(QObject *parent) : KAbstractStyleParameters(parent)
{
    radius = 0;
    CheckBox_Radius = 0;
}



QPalette KClassicalStyleParameters::setPalette(QPalette &palette, bool isDark)
{
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

    if (isDark) {
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

    return palette;
}



QColor KClassicalStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}


KFashionStyleParameters::KFashionStyleParameters(QObject *parent) : KAbstractStyleParameters(parent)
{
    radius = 0;

    ColoseButtonColor = QColor("#F86458");

    Menu_Radius = 0;
    MenuItem_Radius = 0;
}



QPalette KFashionStyleParameters::setPalette(QPalette &palette, bool isDark)
{

    QColor windowText_at(5, 9, 19),
           windowText_iat(5, 9, 19),
           windowText_dis(169, 173, 183),
           button_at(204, 208, 218),
           button_iat(204, 208, 218),
           button_dis(219, 223, 233),
           light_at(239, 243, 253),
           light_iat(239, 243, 253),
           light_dis(239, 243, 253),
           midlight_at(190, 194, 204),
           midlight_iat(190, 194, 204),
           midlight_dis(190, 194, 204),
           dark_at(151, 155, 165),
           dark_iat(151, 155, 165),
           dark_dis(179, 184, 193),
           mid_at(174, 178, 188),
           mid_iat(174, 178, 188),
           mid_dis(174, 178, 188),
           text_at(5, 9, 20),
           text_iat(5, 9, 20),
           text_dis(169, 173, 183),
           brightText_at(238, 242, 253),
           brightText_iat(238, 242, 253),
           brightText_dis(169, 173, 183),
           buttonText_at(5, 9, 20),
           buttonText_iat(5, 9, 20),
           buttonText_dis(169, 173, 183),
           base_at(238, 242, 253),
           base_iat(238, 242, 253),
           base_dis(232, 236, 246),
           window_at(216, 220, 231),
           window_iat(218, 222, 233),
           window_dis(218, 222, 233),
           shadow_at(214, 214, 214),
           shadow_iat(214, 214, 214),
           shadow_dis(201, 201, 201),
           highLight_at(61, 107, 229),
           highLight_iat(61, 107, 229),
           hightight_dis(239, 242, 253),
           highLightText_at(239, 242, 253),
           highLightText_iat(239, 242, 253),
           highLightText_dis(239, 242, 253),
           link_at(5, 9, 253),
           link_iat(5, 9, 253),
           link_dis(5, 9, 253),
           linkVisited_at(239, 9, 253),
           linkVisited_iat(239, 9, 253),
           linkVisited_dis(239, 9, 253),
           alternateBase_at(232, 236, 246),
           alternateBase_iat(232, 236, 246),
           alternateBase_dis(218, 222, 233),
           noRale_at(239, 242, 253),
           noRole_iat(239, 242, 253),
           noRole_dis(239, 242, 253),
           toolTipBase(232, 236, 246),
           toolTipText(25, 29, 40),
           placeholderText(169, 173, 183);

    if (isDark) {
        windowText_at.setRgb(216, 221, 233);
        windowText_iat.setRgb(216, 221, 233);
        windowText_dis.setRgb(77, 77, 77);
        button_at.setRgb(61, 65, 81);
        button_iat.setRgb(61, 65, 81);
        button_dis.setRgb(46, 50, 64);
        light_at.setRgb(236, 241, 252);
        light_iat.setRgb(236, 241, 252);
        light_dis.setRgb(236, 241, 252);
        midlight_at.setRgb(188, 193, 206);
        midlight_iat.setRgb(188, 193, 206);
        midlight_dis.setRgb(188, 193, 206);
        dark_at.setRgb(149, 154, 166);
        dark_iat.setRgb(149, 154, 166);
        dark_dis.setRgb(132, 138, 152);
        mid_at.setRgb(172, 176, 188);
        mid_iat.setRgb(172, 176, 188);
        mid_dis.setRgb(172, 176, 188);
        text_at.setRgb(219, 222, 233);
        text_iat.setRgb(219, 222, 233);
        text_dis.setRgb(77, 77, 77);
        brightText_at.setRgb(219, 222, 233);
        brightText_iat.setRgb(219, 222, 233);
        brightText_dis.setRgb(77, 77, 77);
        buttonText_at.setRgb(219, 222, 233);
        buttonText_iat.setRgb(219, 222, 233);
        buttonText_dis.setRgb(76, 76, 79);
        base_at.setRgb(33, 38, 51);
        base_iat.setRgb(33, 38, 51);
        base_dis.setRgb(40, 46, 62);
        window_at.setRgb(46, 51, 65);
        window_iat.setRgb(46, 51, 65);
        window_dis.setRgb(46, 51, 65);
        hightight_dis.setRgb(46, 46, 48);
        highLightText_at.setRgb(216, 220, 233);
        highLightText_iat.setRgb(216, 220, 233);
        highLightText_dis.setRgb(216, 220, 233);
        link_at.setRgb(29, 19, 252);
        link_iat.setRgb(29, 19, 252);
        link_dis.setRgb(29, 19, 252);
        linkVisited_at.setRgb(236, 15, 252);
        linkVisited_iat.setRgb(236, 15, 252);
        linkVisited_dis.setRgb(236, 15, 252);
        alternateBase_at.setRgb(37, 42, 59);
        alternateBase_iat.setRgb(37, 42, 59);
        alternateBase_dis.setRgb(47, 53, 74);
        noRale_at.setRgb(34, 39, 54);
        noRole_iat.setRgb(34, 39, 54);
        noRole_dis.setRgb(34, 39, 54);
        toolTipBase.setRgb(61, 66, 83);
        toolTipText.setRgb(215, 219, 232);
        placeholderText.setRgb(95, 100, 114);
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

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::ToolTipBase, toolTipBase);

    palette.setColor(QPalette::ToolTipText, toolTipText);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::PlaceholderText, placeholderText);
#endif

    return palette;
}


QColor KFashionStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}
