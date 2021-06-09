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
#ifndef GLOBAL_H
#define GLOBAL_H

#define MOUSE_MARGINS 10

#define UKUI_BEGIN_NAMESPACE namespace ukui_platform_plugin {
#define UKUI_END_NAMESPACE }
#define UKUI_USE_NAMESPACE using namespace ukui_platform_plugin;

#define PUBLIC_CLASS(Class, Target) \
    class U##Class : public Class\
    {friend class Target;}

#define DEFINE_CONST_CHAR(Name) const char Name[] = "ukui_" #Name

DEFINE_CONST_CHAR(useUxcb);
DEFINE_CONST_CHAR(redirectContent);
DEFINE_CONST_CHAR(netWmStates);
DEFINE_CONST_CHAR(windowRadius);
DEFINE_CONST_CHAR(borderWidth);
DEFINE_CONST_CHAR(borderColor);
DEFINE_CONST_CHAR(shadowRadius);
DEFINE_CONST_CHAR(shadowOffset);
DEFINE_CONST_CHAR(shadowColor);
DEFINE_CONST_CHAR(clipPath);
DEFINE_CONST_CHAR(frameMask);
DEFINE_CONST_CHAR(frameMargins);
DEFINE_CONST_CHAR(translucentBackground);
DEFINE_CONST_CHAR(enableSystemResize);
DEFINE_CONST_CHAR(enableSystemMove);
DEFINE_CONST_CHAR(enableBlurWindow);
DEFINE_CONST_CHAR(userWindowMinimumSize);
DEFINE_CONST_CHAR(userWindowMaximumSize);
DEFINE_CONST_CHAR(windowBlurAreas);
DEFINE_CONST_CHAR(windowBlurPaths);
DEFINE_CONST_CHAR(autoInputMaskByClipPath);
DEFINE_CONST_CHAR(popupSystemWindowMenu);
DEFINE_CONST_CHAR(groupLeader);
DEFINE_CONST_CHAR(noTitlebar);
DEFINE_CONST_CHAR(enableGLPaint);

// others
DEFINE_CONST_CHAR(WmWindowTypes);
DEFINE_CONST_CHAR(WmNetDesktop);
DEFINE_CONST_CHAR(WmClass);
DEFINE_CONST_CHAR(ProcessId);


enum DeviceType {
    UnknowDevice,
    TouchapdDevice,
    MouseDevice
};
#endif // GLOBAL_H
