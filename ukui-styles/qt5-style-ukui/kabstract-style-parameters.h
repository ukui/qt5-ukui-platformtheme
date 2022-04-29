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

#ifndef KABSTRACTSTYLEPARAMETERS_H
#define KABSTRACTSTYLEPARAMETERS_H

#include <QObject>
#include <QPalette>
#include <QStyleOption>


class KAbstractStyleParameters
{
public:
    KAbstractStyleParameters(QObject *parent, bool isDark);

    // radius
    int radius = 6;

    // common
    int SmallIcon_Size = 16;
    int IconButton_Distance = 8;

    // button
    int Button_MarginHeight = 2;
    int ToolButton_MarginWidth = 10;
    int Button_MarginWidth = 16;
    int Button_IconSize = 16;
    int Button_IndicatorSize = 16;
    int Button_DefaultIndicatorSize = 0;
    int Button_DefaultWidth = 96;
    int Button_DefaultHeight = 36;
    int ToolButton_DefaultWidth = 60;
    int IconButton_DefaultWidth = 36;
    QColor ColoseButtonColor = QColor(243, 34, 45);

    // menu
    int Menu_MarginHeight = 4 + 5;
    int Menu_Combobox_Popup_MarginHeight = 6;
    int Menu_MarginWidth = 4 + 5;
    int Menu_MarginPanelWidth = 0;
    int Menu_Radius = 8;

    // menu-item
    int MenuItem_MarginHeight = 2;
    int MenuItem_MarginWidth = 12 + 4;
    int MenuItemSeparator_MarginHeight = 4;
    int MenuItemSeparator_MarginWidth = 4;
    int MenuItem_DefaultHeight = 36;
    int MenuItem_DefaultWidght = 152;
    int MenuItem_Radius = 6;

    // scrollbar
    int ScroolBar_Width = 16;
    int ScroolBar_Height = 68;

    // progressbar
    int ProgressBar_DefaultLength = 388;
    int ProgressBar_DefaultThick = 16;

    // slider
    int Slider_DefaultLength = 94;
    int Slider_Length = 20;
    int Slider_GrooveLength = 4;

    // radiobutton
    int ExclusiveIndicator_Width = 16;
    int ExclusiveIndicator_Height = 16;
    int RadioButtonLabel_Spacing = 8;
    int RadioButton_DefaultHeight = 36;
    int RadioButton_OnLength = 6;

    // checkbox
    int Indicator_Width = 16;
    int Indicator_Height = 16;
    int CheckBox_DefaultHeight = 36;
    int CheckBox_Radius = 4;

    // lineedit
    int LineEdit_DefaultWidth = 160;
    int LineEdit_DefaultHeight = 36;

    // combobox
    int ComboBox_DefaultWidth = 160;
    int ComboBox_DefaultHeight = 36;
    int ComboBox_FrameWidth = 2;

    // spinbox
    int SpinBox_DefaultWidth = 160;
    int SpinBox_DefaultHeight = 36;
    int SpinBox_FrameWidth = 2;

    // tabbar
    int TabBar_DefaultWidth = 168;
    int TabBar_DefaultHeight = 40;
    int TabBar_ScrollButtonWidth = 16;
    int TabBar_ScrollButtonOverlap = 2;

    // tooltip
    int ToolTip_DefaultMargin = 8;

    QPalette defaultPalette;




    QPalette setPalette(QPalette &palette);
    virtual QColor radiobutton_default(bool isDark) = 0;// palette::Button

    virtual void initPalette(bool isDark);
};


class KDefaultStyleParameters : public KAbstractStyleParameters
{
public:
    KDefaultStyleParameters(QObject *parent, bool isDark);

    void initPalette(bool isDark) override;

    QColor radiobutton_default(bool isDark) override;
};


class KClassicalStyleParameters : public KAbstractStyleParameters
{
public:
    KClassicalStyleParameters(QObject *parent, bool isDark);

    void initPalette(bool isDark) override;

    QColor radiobutton_default(bool isDark) override;
};


class KFashionStyleParameters : public KAbstractStyleParameters
{
public:
    KFashionStyleParameters(QObject *parent, bool isDark);

    void initPalette(bool isDark) override;

    QColor radiobutton_default(bool isDark) override;
};
#endif // KABSTRACTSTYLEPARAMETERS_H
