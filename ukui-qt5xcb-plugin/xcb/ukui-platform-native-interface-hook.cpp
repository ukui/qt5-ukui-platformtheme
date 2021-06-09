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

#include "ukui-platform-native-interface-hook.h"
#include "ukui-platform-integration.h"

#ifdef Q_OS_LINUX
#include "xcb-native-event-filter.h"
#include "qxcbnativeinterface.h"
typedef QXcbNativeInterface ukuiPlatformNativeInterface;
#elif defined(Q_OS_WIN)
#include "qwindowsgdinativeinterface.h"
typedef QWindowsGdiNativeInterface ukuiPlatformNativeInterface;
#endif

UKUI_BEGIN_NAMESPACE

static QString version()
{
    return QStringLiteral(UXCB_VERSION);
}

#ifdef Q_OS_LINUX
static DeviceType _inputEventSourceDevice(const QInputEvent *event)
{
    return ukuiPlatformIntegration::instance()->eventFilter()->xiEventSource(event);
}
#endif

QFunctionPointer ukuiPlatformNativeInterfaceHook::platformFunction(QPlatformNativeInterface *interface, const QByteArray &function)
{

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    return static_cast<ukuiPlatformNativeInterface*>(interface)->ukuiPlatformNativeInterface::platformFunction(function);
#endif
    return 0;
}

UKUI_END_NAMESPACE
