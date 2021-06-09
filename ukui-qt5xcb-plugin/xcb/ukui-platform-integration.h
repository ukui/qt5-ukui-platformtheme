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
#ifndef UKUIPLATFORMINTEGRATIONPLUGIN_H
#define UKUIPLATFORMINTEGRATIONPLUGIN_H

#include "global.h"

#include <QtGlobal>

#ifdef Q_OS_LINUX
#include "qxcbintegration.h"
typedef QXcbIntegration ukuiPlatformIntegrationParent;
class QXcbVirtualDesktop;
#elif defined(Q_OS_WIN)
#include "qwindowsgdiintegration.h"
typedef QWindowsGdiIntegration ukuiPlatformIntegrationParent;
#endif

UKUI_BEGIN_NAMESPACE



class ukuiPlatformBackingStoreHelper;
class XcbNativeEventFilter;
class ukuiXcbXSettings;

class ukuiPlatformIntegration : public ukuiPlatformIntegrationParent
{
public:
    ukuiPlatformIntegration(const QStringList &parameters, int &argc, char **argv);
    ~ukuiPlatformIntegration();

    QPlatformWindow *createPlatformWindow(QWindow *window) const Q_DECL_OVERRIDE;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const Q_DECL_OVERRIDE;
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const Q_DECL_OVERRIDE;
    QPaintEngine *createImagePaintEngine(QPaintDevice *paintDevice) const override;

//    QStringList themeNames() const Q_DECL_OVERRIDE;
//    QVariant styleHint(StyleHint hint) const override;

    void initialize() Q_DECL_OVERRIDE;

#ifdef Q_OS_LINUX

    inline static ukuiPlatformIntegration *instance()
    { return static_cast<ukuiPlatformIntegration*>(ukuiPlatformIntegrationParent::instance());}

    inline static QXcbConnection *xcbConnection()
    { return instance()->defaultConnection();}

    inline XcbNativeEventFilter *eventFilter()
    { return m_eventFilter;}

    bool enableCursorBlink() const;
    ukuiXcbXSettings *xSettings(bool onlyExists = false) const;
    static ukuiXcbXSettings *xSettings(QXcbConnection *connection);

private:
    XcbNativeEventFilter *m_eventFilter = Q_NULLPTR;
    static ukuiXcbXSettings *m_xsettings;
#endif

};

UKUI_END_NAMESPACE

#endif // UKUIPLATFORMINTEGRATIONPLUGIN_H
