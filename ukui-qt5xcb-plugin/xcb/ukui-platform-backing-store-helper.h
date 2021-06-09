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
#ifndef UKUIPLATFORMBACKINGSTOREHELPER_H
#define UKUIPLATFORMBACKINGSTOREHELPER_H

#include <QtGlobal>

#include "global.h"

QT_BEGIN_NAMESPACE
class QPlatformBackingStore;
class QWindow;
class QRegion;
class QPoint;
class QPaintDevice;
class QSize;
QT_END_NAMESPACE

UKUI_BEGIN_NAMESPACE

class ukuiPlatformBackingStoreHelper
{
public:
    ukuiPlatformBackingStoreHelper();

    bool addBackingStore(QPlatformBackingStore *store);

    QPlatformBackingStore *backingStore() const
    { return reinterpret_cast<QPlatformBackingStore*>(const_cast<ukuiPlatformBackingStoreHelper*>(this));}

    QPaintDevice *paintDevice();
    void beginPaint(const QRegion &);
    void flush(QWindow *window, const QRegion &region, const QPoint &offset);
#ifdef Q_OS_LINUX
    void resize(const QSize &size, const QRegion &staticContents);
#endif

};

UKUI_END_NAMESPACE

#endif // UKUIPLATFORMBACKINGSTOREHELPER_H
