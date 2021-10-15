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



class KAbstractStyleParameters
{
public:
    KAbstractStyleParameters(QObject *parent);

    // palette
    virtual QPalette setPalette(QPalette &palette, bool isDark) = 0;

    // palette::HighLight
    QColor HL_Blue_Default = QColor(55, 144, 250);
    QColor HL_Blue_Hover = QColor(55, 144, 250);
    QColor HL_Blue_Click = QColor(36, 109, 212);
    // palette::Button
    virtual QColor button_default(bool isDark) = 0;
    virtual QColor button_hover(bool isDark) = 0;
    virtual QColor button_click(bool isDark) = 0;

    // radius
    int radius = 6;

    // common
    int SmallIcon_Size = 16;

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
    QColor ColoseButton_Hover = QColor(243, 34, 45);
    QColor ColoseButton_Click = QColor(204, 18, 34);

    // menu
    int Menu_MarginHeight = 4 + 5;
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
};



class KFashionStyleParameters : public KAbstractStyleParameters
{
public:
    KFashionStyleParameters(QObject *parent);

    QPalette setPalette(QPalette &palette, bool isDark) override;

    QColor button_default(bool isDark) override;
    QColor button_hover(bool isDark) override;
    QColor button_click(bool isDark) override;
};



class KDefaultStyleParameters : public KAbstractStyleParameters
{
public:
    KDefaultStyleParameters(QObject *parent);

    QPalette setPalette(QPalette &palette, bool isDark) override;

    QColor button_default(bool isDark) override;
    QColor button_hover(bool isDark) override;
    QColor button_click(bool isDark) override;
};



class KClassicalStyleParameters : public KAbstractStyleParameters
{
public:
    KClassicalStyleParameters(QObject *parent);

    QPalette setPalette(QPalette &palette, bool isDark) override;

    QColor button_default(bool isDark) override;
    QColor button_hover(bool isDark) override;
    QColor button_click(bool isDark) override;
};

#endif // KABSTRACTSTYLEPARAMETERS_H
