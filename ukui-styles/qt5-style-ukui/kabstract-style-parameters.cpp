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


#include <QPushButton>
#include <QToolButton>
#include <QApplication>


#include "kabstract-style-parameters.h"
#include "black-list.h"

extern QColor mixColor(const QColor &c1, const QColor &c2, qreal bias);

KAbstractStyleParameters::KAbstractStyleParameters(QObject *parent, bool isDark)
{

}

QPalette KAbstractStyleParameters::setPalette(QPalette &palette)
{
    palette.setColor(QPalette::Active, QPalette::WindowText, defaultPalette.color(QPalette::Active, QPalette::WindowText));
    palette.setColor(QPalette::Inactive, QPalette::WindowText, defaultPalette.color(QPalette::Inactive, QPalette::WindowText));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));

    palette.setColor(QPalette::Active, QPalette::Button, defaultPalette.color(QPalette::Active, QPalette::Button));
    palette.setColor(QPalette::Inactive, QPalette::Button, defaultPalette.color(QPalette::Inactive, QPalette::Button));
    palette.setColor(QPalette::Disabled, QPalette::Button, defaultPalette.color(QPalette::Disabled, QPalette::Button));

    palette.setColor(QPalette::Active, QPalette::Light, defaultPalette.color(QPalette::Active, QPalette::Light));
    palette.setColor(QPalette::Inactive, QPalette::Light, defaultPalette.color(QPalette::Inactive, QPalette::Light));
    palette.setColor(QPalette::Disabled, QPalette::Light, defaultPalette.color(QPalette::Disabled, QPalette::Light));

    palette.setColor(QPalette::Active, QPalette::Midlight, defaultPalette.color(QPalette::Active, QPalette::Midlight));
    palette.setColor(QPalette::Inactive, QPalette::Midlight, defaultPalette.color(QPalette::Inactive, QPalette::Midlight));
    palette.setColor(QPalette::Disabled, QPalette::Midlight, defaultPalette.color(QPalette::Disabled, QPalette::Midlight));

    palette.setColor(QPalette::Active, QPalette::Dark, defaultPalette.color(QPalette::Active, QPalette::Dark));
    palette.setColor(QPalette::Inactive, QPalette::Dark, defaultPalette.color(QPalette::Inactive, QPalette::Dark));
    palette.setColor(QPalette::Disabled, QPalette::Dark, defaultPalette.color(QPalette::Disabled, QPalette::Dark));

    palette.setColor(QPalette::Active, QPalette::Mid, defaultPalette.color(QPalette::Active, QPalette::Mid));
    palette.setColor(QPalette::Inactive, QPalette::Mid, defaultPalette.color(QPalette::Inactive, QPalette::Mid));
    palette.setColor(QPalette::Disabled, QPalette::Mid, defaultPalette.color(QPalette::Disabled, QPalette::Mid));

    palette.setColor(QPalette::Active, QPalette::Text, defaultPalette.color(QPalette::Active, QPalette::Text));
    palette.setColor(QPalette::Inactive, QPalette::Text, defaultPalette.color(QPalette::Inactive, QPalette::Text));
    palette.setColor(QPalette::Disabled, QPalette::Text, defaultPalette.color(QPalette::Disabled, QPalette::Text));

    palette.setColor(QPalette::Active, QPalette::BrightText, defaultPalette.color(QPalette::Active, QPalette::BrightText));
    palette.setColor(QPalette::Inactive, QPalette::BrightText, defaultPalette.color(QPalette::Inactive, QPalette::BrightText));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, defaultPalette.color(QPalette::Disabled, QPalette::BrightText));

    palette.setColor(QPalette::Active, QPalette::ButtonText, defaultPalette.color(QPalette::Active, QPalette::ButtonText));
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, defaultPalette.color(QPalette::Inactive, QPalette::ButtonText));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, defaultPalette.color(QPalette::Disabled, QPalette::ButtonText));

    palette.setColor(QPalette::Active, QPalette::Base, defaultPalette.color(QPalette::Active, QPalette::Base));
    palette.setColor(QPalette::Inactive, QPalette::Base, defaultPalette.color(QPalette::Inactive, QPalette::Base));
    palette.setColor(QPalette::Disabled, QPalette::Base, defaultPalette.color(QPalette::Disabled, QPalette::Base));

    palette.setColor(QPalette::Active, QPalette::Window, defaultPalette.color(QPalette::Active, QPalette::Window));
    palette.setColor(QPalette::Inactive, QPalette::Window, defaultPalette.color(QPalette::Inactive, QPalette::Window));
    palette.setColor(QPalette::Disabled, QPalette::Window, defaultPalette.color(QPalette::Disabled, QPalette::Window));

    palette.setColor(QPalette::Active, QPalette::Shadow, defaultPalette.color(QPalette::Active, QPalette::Shadow));
    palette.setColor(QPalette::Inactive, QPalette::Shadow, defaultPalette.color(QPalette::Inactive, QPalette::Shadow));
    palette.setColor(QPalette::Disabled, QPalette::Shadow, defaultPalette.color(QPalette::Disabled, QPalette::Shadow));

    palette.setColor(QPalette::Active, QPalette::Highlight, defaultPalette.color(QPalette::Active, QPalette::Highlight));
    palette.setColor(QPalette::Inactive, QPalette::Highlight, defaultPalette.color(QPalette::Inactive, QPalette::Highlight));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, defaultPalette.color(QPalette::Disabled, QPalette::Highlight));

    palette.setColor(QPalette::Active, QPalette::HighlightedText, defaultPalette.color(QPalette::Active, QPalette::HighlightedText));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, defaultPalette.color(QPalette::Inactive, QPalette::HighlightedText));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, defaultPalette.color(QPalette::Disabled, QPalette::HighlightedText));

    palette.setColor(QPalette::Active, QPalette::Link, defaultPalette.color(QPalette::Active, QPalette::Link));
    palette.setColor(QPalette::Inactive, QPalette::Link, defaultPalette.color(QPalette::Inactive, QPalette::Link));
    palette.setColor(QPalette::Disabled, QPalette::Link, defaultPalette.color(QPalette::Disabled, QPalette::Link));

    palette.setColor(QPalette::Active, QPalette::LinkVisited, defaultPalette.color(QPalette::Active, QPalette::LinkVisited));
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, defaultPalette.color(QPalette::Inactive, QPalette::LinkVisited));
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, defaultPalette.color(QPalette::Disabled, QPalette::LinkVisited));

    palette.setColor(QPalette::Active, QPalette::AlternateBase, defaultPalette.color(QPalette::Active, QPalette::AlternateBase));
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, defaultPalette.color(QPalette::Inactive, QPalette::AlternateBase));
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, defaultPalette.color(QPalette::Disabled, QPalette::AlternateBase));

    palette.setColor(QPalette::Active, QPalette::NoRole, defaultPalette.color(QPalette::Active, QPalette::NoRole));
    palette.setColor(QPalette::Inactive, QPalette::NoRole, defaultPalette.color(QPalette::Inactive, QPalette::NoRole));
    palette.setColor(QPalette::Disabled, QPalette::NoRole, defaultPalette.color(QPalette::Disabled, QPalette::NoRole));

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, defaultPalette.color(QPalette::Active, QPalette::ToolTipBase));
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, defaultPalette.color(QPalette::Inactive, QPalette::ToolTipBase));
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, defaultPalette.color(QPalette::Disabled, QPalette::ToolTipBase));

    palette.setColor(QPalette::Active, QPalette::ToolTipText, defaultPalette.color(QPalette::Active, QPalette::ToolTipText));
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, defaultPalette.color(QPalette::Inactive, QPalette::ToolTipText));
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, defaultPalette.color(QPalette::Disabled, QPalette::ToolTipText));

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, defaultPalette.color(QPalette::Active, QPalette::PlaceholderText));
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, defaultPalette.color(QPalette::Inactive, QPalette::PlaceholderText));
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, defaultPalette.color(QPalette::Disabled, QPalette::PlaceholderText));
#endif

    return palette;
}


//Default theme
KDefaultStyleParameters::KDefaultStyleParameters(QObject *parent, bool isDark) : KAbstractStyleParameters(parent, isDark)
{
    radius = 6;

    ColoseButtonColor = QColor(243, 34, 45);

    Menu_Radius = 8;
    MenuItem_Radius = 6;

    initPalette(isDark);
}

void KDefaultStyleParameters::initPalette(bool isDark)
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

    defaultPalette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Button, button_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Light, light_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Text, text_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Base, base_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Window, window_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Highlight, highLight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Link, link_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    defaultPalette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif
}


void KDefaultStyleParameters::initPushButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    pushButtonParameters.radius = 6;

    QBrush defaultBrush;
    QBrush clickBrush;
    QBrush hoverBrush;
    QBrush disableBrush;
    QBrush disableOnBrush;


    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = option->palette.color(QPalette::Active, QPalette::BrightText);
    if (isDark) {
        hoverBrush = QBrush(mixColor(highlight, mix, 0.2));
        clickBrush = QBrush(mixColor(highlight, mix, 0.05));
    } else {
        hoverBrush = QBrush(mixColor(highlight, mix, 0.05));
        clickBrush = QBrush(mixColor(highlight, mix, 0.2));
    }
    defaultBrush = option->palette.brush(QPalette::Active, QPalette::Button);
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disableOnBrush = option->palette.brush(QPalette::Disabled, QPalette::NoRole);

    if (qobject_cast<const QPushButton *>(widget)) {
        bool isWindowButton = false;
        bool isWindowColoseButton = false;
        bool isImportant = false;
        bool useButtonPalette = false;

        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (isWindowColoseButton) {
            QColor ColoseButton = ColoseButtonColor;

            if (isDark) {
                hoverBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
            } else {
                hoverBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
            }
        } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
            if (isDark) {
                mix.setAlphaF(0.28);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.15);
                clickBrush = QBrush(mix);
            } else {
                mix.setAlphaF(0.15);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.28);
                clickBrush = QBrush(mix);
            }
        } else if (useButtonPalette || isWindowButton) {
            QColor button = option->palette.color(QPalette::Active, QPalette::Button);

            if (isDark) {
                hoverBrush = QBrush(mixColor(button, mix, 0.2));
                clickBrush = QBrush(mixColor(button, mix, 0.05));
            } else {
                hoverBrush = QBrush(mixColor(button, mix, 0.05));
                clickBrush = QBrush(mixColor(button, mix, 0.2));
            }
        }

        if (isImportant) {
            defaultBrush = highlight;
        }
    }

    pushButtonParameters.pushButtonDefaultBrush   = defaultBrush;
    pushButtonParameters.pushButtonClickBrush     = clickBrush;
    pushButtonParameters.pushButtonHoverBrush     = hoverBrush;
    pushButtonParameters.pushButtonDisableBrush   = disableBrush;
    pushButtonParameters.pushButtonDisableOnBrush = disableOnBrush;
}

void KDefaultStyleParameters::initToolButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    toolButtonParameters.radius = 6;

    QBrush defaultBrush;
    QBrush clickBrush;
    QBrush hoverBrush;
    QBrush disableBrush;
    QBrush disableOnBrush;

    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = option->palette.color(QPalette::Active, QPalette::BrightText);
    if (isDark) {
        hoverBrush = QBrush(mixColor(highlight, mix, 0.2));
        clickBrush = QBrush(mixColor(highlight, mix, 0.05));
    } else {
        hoverBrush = QBrush(mixColor(highlight, mix, 0.05));
        clickBrush = QBrush(mixColor(highlight, mix, 0.2));
    }
    defaultBrush = option->palette.brush(QPalette::Active, QPalette::Button);
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disableOnBrush = option->palette.brush(QPalette::Disabled, QPalette::NoRole);

    if (qobject_cast<const QToolButton *>(widget)) {
        bool isWindowButton = false;
        bool isWindowColoseButton = false;
        bool useButtonPalette = false;

        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (isWindowColoseButton) {
            QColor ColoseButton = ColoseButtonColor;

            if (isDark) {
                hoverBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
            } else {
                hoverBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
            }
        } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
            if (isDark) {
                mix.setAlphaF(0.28);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.15);
                clickBrush = QBrush(mix);
            } else {
                mix.setAlphaF(0.15);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.28);
                clickBrush = QBrush(mix);
            }
        } else if (useButtonPalette || isWindowButton) {
            QColor button = option->palette.color(QPalette::Active, QPalette::Button);

            if (isDark) {
                hoverBrush = QBrush(mixColor(button, mix, 0.2));
                clickBrush = QBrush(mixColor(button, mix, 0.05));
            } else {
                hoverBrush = QBrush(mixColor(button, mix, 0.05));
                clickBrush = QBrush(mixColor(button, mix, 0.2));
            }
        }
    }

    toolButtonParameters.toolButtonDefaultBrush   = defaultBrush;
    toolButtonParameters.toolButtonClickBrush     = clickBrush;
    toolButtonParameters.toolButtonHoverBrush     = hoverBrush;
    toolButtonParameters.toolButtonDisableBrush   = disableBrush;
    toolButtonParameters.toolButtonDisableOnBrush = disableOnBrush;
}

void KDefaultStyleParameters::initLineEditParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    lineEditParameters.radius = 6;

    QBrush defaultBrush;
    QBrush hoverBrush;
    QBrush focusBrush;
    QBrush disableBrush;

    QPen defaultPen;
    QPen hoverPen;
    QPen focusPen;
    QPen disablePen;

    //default state
    defaultBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    defaultPen = QPen(Qt::NoPen);

    //hover state
    hoverBrush = defaultBrush;
    hoverPen = defaultPen;

    //focus state
    focusBrush = option->palette.brush(QPalette::Active, QPalette::Base);
    focusPen = QPen(option->palette.brush(QPalette::Active, QPalette::Highlight),
                    2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //disable state
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disablePen = QPen(Qt::NoPen);


    lineEditParameters.lineEditDefaultBrush = defaultBrush;
    lineEditParameters.lineEditHoverBrush = hoverBrush;;
    lineEditParameters.lineEditFocusBrush = focusBrush;;
    lineEditParameters.lineEditDisableBrush = disableBrush;

    lineEditParameters.lineEditDefaultPen = defaultPen;
    lineEditParameters.lineEditHoverPen = hoverPen;
    lineEditParameters.lineEditFocusPen = focusPen;
    lineEditParameters.lineEditDisablePen = disablePen;
}

void KDefaultStyleParameters::initSpinBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    spinBoxParameters.radius = 6;

    QBrush defaultBrush;
    QBrush hoverBrush;
    QBrush focusBrush;
    QBrush disableBrush;

    QPen defaultPen;
    QPen hoverPen;
    QPen focusPen;
    QPen disablePen;

    //default state
    defaultBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    defaultPen = QPen(Qt::NoPen);

    //hover state
    hoverBrush = defaultBrush;
    hoverPen = defaultPen;

    //focus state
    focusBrush = option->palette.brush(QPalette::Active, QPalette::Base);
    focusPen = QPen(option->palette.brush(QPalette::Active, QPalette::Highlight),
                    2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //disable state
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disablePen = QPen(Qt::NoPen);


    spinBoxParameters.spinBoxDefaultBrush = defaultBrush;
    spinBoxParameters.spinBoxHoverBrush = hoverBrush;;
    spinBoxParameters.spinBoxFocusBrush = focusBrush;;
    spinBoxParameters.spinBoxDisableBrush = disableBrush;

    spinBoxParameters.spinBoxDefaultPen = defaultPen;
    spinBoxParameters.spinBoxHoverPen = hoverPen;
    spinBoxParameters.spinBoxFocusPen = focusPen;
    spinBoxParameters.spinBoxDisablePen = disablePen;
}

QColor KDefaultStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}


//Classical theme
KClassicalStyleParameters::KClassicalStyleParameters(QObject *parent, bool isDark) : KAbstractStyleParameters(parent, isDark)
{
    radius = 0;
    CheckBox_Radius = 0;

    initPalette(isDark);
}

void KClassicalStyleParameters::initPalette(bool isDark)
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

    defaultPalette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Button, button_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Light, light_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Text, text_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Base, base_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Window, window_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Highlight, hightight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Link, link_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    defaultPalette.setColor(QPalette::AlternateBase, alternateBase_at);

    defaultPalette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    defaultPalette.setColor(QPalette::ToolTipBase, toolTipBase);

    defaultPalette.setColor(QPalette::ToolTipText, toolTipText);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    defaultPalette.setColor(QPalette::PlaceholderText, placeholderText);
#endif
}

void KClassicalStyleParameters::initPushButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{

}

void KClassicalStyleParameters::initToolButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{

}

void KClassicalStyleParameters::initLineEditParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{

}

void KClassicalStyleParameters::initSpinBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{

}

QColor KClassicalStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}


KFashionStyleParameters::KFashionStyleParameters(QObject *parent, bool isDark) : KAbstractStyleParameters(parent, isDark)
{
    radius = 0;

    ColoseButtonColor = QColor("#F86458");

    Menu_Radius = 0;
    MenuItem_Radius = 0;

    initPalette(isDark);
}

void KFashionStyleParameters::initPalette(bool isDark)
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
           brightText_at(0, 0, 0),
           brightText_iat(0, 0, 0),
           brightText_dis(0, 0, 0),
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
        brightText_at.setRgb(255, 255, 255);
        brightText_iat.setRgb(255, 255, 255);
        brightText_dis.setRgb(255, 255, 255);
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

    defaultPalette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Button, button_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Light, light_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Text, text_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Base, base_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Window, window_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Highlight, hightight_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::Link, link_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    defaultPalette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    defaultPalette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    defaultPalette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    defaultPalette.setColor(QPalette::ToolTipBase, toolTipBase);

    defaultPalette.setColor(QPalette::ToolTipText, toolTipText);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    defaultPalette.setColor(QPalette::PlaceholderText, placeholderText);
#endif

}

void KFashionStyleParameters::initPushButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    pushButtonParameters.radius = 6;

    QBrush defaultBrush;
    QBrush clickBrush;
    QBrush hoverBrush;
    QBrush disableBrush;
    QBrush disableOnBrush;

    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = option->palette.color(QPalette::Active, QPalette::BrightText);

    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;
    linearGradient.setStart(option->rect.topLeft());
    linearGradient.setFinalStop(option->rect.bottomLeft());
    if (isDark) {
        //hover
        startColor = mixColor(highlight, QColor(Qt::white), 0.2);
        endColor = highlight;
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        hoverBrush = QBrush(linearGradient);

        //click
        clickBrush = QBrush(mixColor(highlight, mix, 0.05));
    } else {
        //hover
        startColor = mixColor(highlight, QColor(Qt::white), 0.2);
        endColor = mixColor(highlight, QColor(Qt::black), 0.05);
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        hoverBrush = QBrush(linearGradient);

        //click
        clickBrush = QBrush(mixColor(highlight, mix, 0.2));
    }
    defaultBrush = option->palette.brush(QPalette::Active, QPalette::Button);
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disableOnBrush = option->palette.brush(QPalette::Disabled, QPalette::NoRole);

    if (qobject_cast<const QPushButton *>(widget)) {
        bool isWindowButton = false;
        bool isWindowColoseButton = false;
        bool isImportant = false;
        bool useButtonPalette = false;

        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }
        if (widget->property("isImportant").isValid()) {
            isImportant = widget->property("isImportant").toBool();
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (isWindowColoseButton) {
            QColor ColoseButton = ColoseButtonColor;

            if (isDark) {
                //hover
                startColor = mixColor(ColoseButton, QColor(Qt::white), 0.2);
                endColor = ColoseButton;
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
            } else {
                //hover
                startColor = mixColor(ColoseButton, QColor(Qt::white), 0.2);
                endColor = mixColor(ColoseButton, QColor(Qt::black), 0.05);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
            }
        } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
            if (isDark) {
                mix.setAlphaF(0.28);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.15);
                clickBrush = QBrush(mix);
            } else {
                mix.setAlphaF(0.15);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.28);
                clickBrush = QBrush(mix);
            }
        } else if (useButtonPalette || isWindowButton) {
            QColor button = option->palette.color(QPalette::Active, QPalette::Button);

            if (isDark) {
                //hover
                startColor = mixColor(button, QColor(Qt::white), 0.2);
                endColor = mixColor(button, QColor(Qt::white), 0.1);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(button, mix, 0.05));
            } else {
                //hover
                startColor = mixColor(button, QColor(Qt::black), 0.1);
                endColor = mixColor(button, QColor(Qt::black), 0.2);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(button, mix, 0.2));
            }
        }

        if (isImportant) {
            defaultBrush = highlight;
        }
    }

    pushButtonParameters.pushButtonDefaultBrush   = defaultBrush;
    pushButtonParameters.pushButtonClickBrush     = clickBrush;
    pushButtonParameters.pushButtonHoverBrush     = hoverBrush;
    pushButtonParameters.pushButtonDisableBrush   = disableBrush;
    pushButtonParameters.pushButtonDisableOnBrush = disableOnBrush;
}

void KFashionStyleParameters::initToolButtonParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    toolButtonParameters.radius = 6;

    QBrush defaultBrush;
    QBrush clickBrush;
    QBrush hoverBrush;
    QBrush disableBrush;
    QBrush disableOnBrush;

    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix = option->palette.color(QPalette::Active, QPalette::BrightText);

    QColor startColor;
    QColor endColor;
    QLinearGradient linearGradient;
    linearGradient.setStart(option->rect.topLeft());
    linearGradient.setFinalStop(option->rect.bottomLeft());
    if (isDark) {
        //hover
        startColor = mixColor(highlight, QColor(Qt::white), 0.2);
        endColor = highlight;
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        hoverBrush = QBrush(linearGradient);

        //click
        clickBrush = QBrush(mixColor(highlight, mix, 0.05));
    } else {
        //hover
        startColor = mixColor(highlight, QColor(Qt::white), 0.2);
        endColor = mixColor(highlight, QColor(Qt::black), 0.05);
        linearGradient.setColorAt(0, startColor);
        linearGradient.setColorAt(1, endColor);
        hoverBrush = QBrush(linearGradient);

        //click
        clickBrush = QBrush(mixColor(highlight, mix, 0.2));
    }
    defaultBrush = option->palette.brush(QPalette::Active, QPalette::Button);
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disableOnBrush = option->palette.brush(QPalette::Disabled, QPalette::NoRole);

    if (qobject_cast<const QToolButton *>(widget)) {
        bool isWindowButton = false;
        bool isWindowColoseButton = false;
        bool useButtonPalette = false;

        if (widget->property("isWindowButton").isValid()) {
            if (widget->property("isWindowButton").toInt() == 0x01) {
                isWindowButton = true;
            }
            if (widget->property("isWindowButton").toInt() == 0x02) {
                isWindowColoseButton = true;
            }
        }

        if (widget->property("useButtonPalette").isValid()) {
            useButtonPalette = widget->property("useButtonPalette").toBool();
        }

        if (isWindowColoseButton) {
            QColor ColoseButton = ColoseButtonColor;

            if (isDark) {
                //hover
                startColor = mixColor(ColoseButton, QColor(Qt::white), 0.2);
                endColor = ColoseButton;
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.05));
            } else {
                //hover
                startColor = mixColor(ColoseButton, QColor(Qt::white), 0.2);
                endColor = mixColor(ColoseButton, QColor(Qt::black), 0.05);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(ColoseButton, mix, 0.2));
            }
        } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
            if (isDark) {
                mix.setAlphaF(0.28);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.15);
                clickBrush = QBrush(mix);
            } else {
                mix.setAlphaF(0.15);
                hoverBrush = QBrush(mix);
                mix.setAlphaF(0.28);
                clickBrush = QBrush(mix);
            }
        } else if (useButtonPalette || isWindowButton) {
            QColor button = option->palette.color(QPalette::Active, QPalette::Button);

            if (isDark) {
                //hover
                startColor = mixColor(button, QColor(Qt::white), 0.2);
                endColor = mixColor(button, QColor(Qt::white), 0.1);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(button, mix, 0.05));
            } else {
                //hover
                startColor = mixColor(button, QColor(Qt::black), 0.1);
                endColor = mixColor(button, QColor(Qt::black), 0.2);
                linearGradient.setColorAt(0, startColor);
                linearGradient.setColorAt(1, endColor);
                hoverBrush = QBrush(linearGradient);

                //click
                clickBrush = QBrush(mixColor(button, mix, 0.2));
            }
        }
    }

    toolButtonParameters.toolButtonDefaultBrush   = defaultBrush;
    toolButtonParameters.toolButtonClickBrush     = clickBrush;
    toolButtonParameters.toolButtonHoverBrush     = hoverBrush;
    toolButtonParameters.toolButtonDisableBrush   = disableBrush;
    toolButtonParameters.toolButtonDisableOnBrush = disableOnBrush;
}

void KFashionStyleParameters::initLineEditParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    lineEditParameters.radius = 6;

    QBrush defaultBrush;
    QBrush hoverBrush;
    QBrush focusBrush;
    QBrush disableBrush;

    QPen defaultPen;
    QPen hoverPen;
    QPen focusPen;
    QPen disablePen;

    //default state
    defaultBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    defaultPen = QPen(Qt::NoPen);

    //hover state
    hoverBrush = defaultBrush;
    hoverPen = defaultPen;

    //focus state
    focusBrush = option->palette.brush(QPalette::Active, QPalette::Base);
    focusPen = QPen(option->palette.brush(QPalette::Active, QPalette::Highlight),
                    1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //disable state
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disablePen = QPen(Qt::NoPen);


    lineEditParameters.lineEditDefaultBrush = defaultBrush;
    lineEditParameters.lineEditHoverBrush = hoverBrush;;
    lineEditParameters.lineEditFocusBrush = focusBrush;;
    lineEditParameters.lineEditDisableBrush = disableBrush;

    lineEditParameters.lineEditDefaultPen = defaultPen;
    lineEditParameters.lineEditHoverPen = hoverPen;
    lineEditParameters.lineEditFocusPen = focusPen;
    lineEditParameters.lineEditDisablePen = disablePen;
}

void KFashionStyleParameters::initSpinBoxParameters(bool isDark, const QStyleOption *option, const QWidget *widget)
{
    spinBoxParameters.radius = 6;

    QBrush defaultBrush;
    QBrush hoverBrush;
    QBrush focusBrush;
    QBrush disableBrush;

    QPen defaultPen;
    QPen hoverPen;
    QPen focusPen;
    QPen disablePen;

    //default state
    defaultBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    defaultPen = QPen(Qt::NoPen);

    //hover state
    hoverBrush = defaultBrush;
    hoverPen = defaultPen;

    //focus state
    focusBrush = option->palette.brush(QPalette::Active, QPalette::Base);
    focusPen = QPen(option->palette.brush(QPalette::Active, QPalette::Highlight),
                    1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    //disable state
    disableBrush = option->palette.brush(QPalette::Disabled, QPalette::Button);
    disablePen = QPen(Qt::NoPen);


    spinBoxParameters.spinBoxDefaultBrush = defaultBrush;
    spinBoxParameters.spinBoxHoverBrush = hoverBrush;;
    spinBoxParameters.spinBoxFocusBrush = focusBrush;;
    spinBoxParameters.spinBoxDisableBrush = disableBrush;

    spinBoxParameters.spinBoxDefaultPen = defaultPen;
    spinBoxParameters.spinBoxHoverPen = hoverPen;
    spinBoxParameters.spinBoxFocusPen = focusPen;
    spinBoxParameters.spinBoxDisablePen = disablePen;
}

QColor KFashionStyleParameters::radiobutton_default(bool isDark)
{
    if (isDark) {
        return QColor(77, 77, 77);
    } else {
        return QColor(166, 166, 166);
    }
}
