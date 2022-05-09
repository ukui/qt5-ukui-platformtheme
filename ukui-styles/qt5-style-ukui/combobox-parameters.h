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
 * Authors: Xibowen <lanyue@kylinos.cn>
 *
 */

#ifndef COMBOBOXPARAMETERS_H
#define COMBOBOXPARAMETERS_H

#include <QBrush>
#include <QPen>

class ComboBoxParameters
{
public:
    ComboBoxParameters();


//private:
    int radius;
    QBrush comboBoxDefaultBrush;
    QBrush comboBoxHoverBrush;
    QBrush comboBoxOnBrush;
    QBrush comboBoxEditBrush;
    QBrush comboBoxDisableBrush;

    QPen comboBoxDefaultPen;
    QPen comboBoxHoverPen;
    QPen comboBoxOnPen;
    QPen comboBoxEditPen;
    QPen comboBoxDisablePen;
};

#endif // COMBOBOXPARAMETERS_H
