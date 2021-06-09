/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UKUIPLATFORMNATIVEINTERFACEHOOK_H
#define UKUIPLATFORMNATIVEINTERFACEHOOK_H
#include <QtGlobal>

#include "global.h"

QT_BEGIN_NAMESPACE
class QPlatformNativeInterface;
QT_END_NAMESPACE

UKUI_BEGIN_NAMESPACE

class ukuiPlatformNativeInterfaceHook
{
public:
    static QFunctionPointer platformFunction(QPlatformNativeInterface *interface, const QByteArray &function);
};

UKUI_END_NAMESPACE

#endif // UKUIPLATFORMNATIVEINTERFACEHOOK_H
