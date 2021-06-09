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
#ifndef UKUIHIGHDPI_H
#define UKUIHIGHDPI_H

#include <QPointF>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>

#include "global.h"

QT_BEGIN_NAMESPACE
class QWindow;
class QXcbScreen;
class QXcbVirtualDesktop;
class QPlatformWindow;
class QXcbBackingStore;
QT_END_NAMESPACE

typedef struct xcb_connection_t xcb_connection_t;

UKUI_BEGIN_NAMESPACE

class ukuiHighDpi
{
public:
    static QPointF fromNativePixels(const QPointF &pixelPoint, const QWindow *window);

    static void init();
    static bool isActive();
    static bool overrideBackingStore();
    static QDpi logicalDpi(QXcbScreen *s);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    static qreal pixelDensity(QXcbScreen *s);
#endif
    static qreal devicePixelRatio(QPlatformWindow *w);
    static void onDPIChanged(xcb_connection_t *screen, const QByteArray &name, const QVariant &property, void *handle);

private:

    static bool active;
    static QHash<QPlatformScreen*, qreal> screenFactorMap;
};

UKUI_END_NAMESPACE

#endif // UKUIHIGHDPI_H
