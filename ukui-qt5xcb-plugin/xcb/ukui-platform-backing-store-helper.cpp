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

#include "ukui-platform-backing-store-helper.h"
#include "vtablehook.h"

#ifdef Q_OS_LINUX
#define private public
#include "qxcbbackingstore.h"
#undef private
#endif

#include <qpa/qplatformbackingstore.h>

#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QThreadStorage>
#include <QThread>

#include <xcb/xcb_image.h>

#ifdef Q_OS_LINUX
QT_BEGIN_NAMESPACE
class QXcbShmImage : public QXcbObject
{
public:
    xcb_shm_segment_info_t m_shm_info;
};
QT_END_NAMESPACE
#endif

UKUI_BEGIN_NAMESPACE

ukuiPlatformBackingStoreHelper::ukuiPlatformBackingStoreHelper()
{

}

bool ukuiPlatformBackingStoreHelper::addBackingStore(QPlatformBackingStore *store)
{
    VtableHook::overrideVfptrFun(store, &QPlatformBackingStore::beginPaint, this, &ukuiPlatformBackingStoreHelper::beginPaint);
    VtableHook::overrideVfptrFun(store, &QPlatformBackingStore::paintDevice, this, &ukuiPlatformBackingStoreHelper::paintDevice);
    VtableHook::overrideVfptrFun(store, &QPlatformBackingStore::resize, this, &ukuiPlatformBackingStoreHelper::resize);

    return VtableHook::overrideVfptrFun(store, &QPlatformBackingStore::flush, this, &ukuiPlatformBackingStoreHelper::flush);
}

static QThreadStorage<bool> ukui_xcb_overridePaintDevice;

QPaintDevice *ukuiPlatformBackingStoreHelper::paintDevice()
{
    QPlatformBackingStore *store = backingStore();

    if (ukui_xcb_overridePaintDevice.hasLocalData() && ukui_xcb_overridePaintDevice.localData()) {
        static thread_local QImage device(1, 1, QImage::Format_Alpha8);

        return &device;
    }

    return VtableHook::callOriginalFun(store, &QPlatformBackingStore::paintDevice);
}

void ukuiPlatformBackingStoreHelper::beginPaint(const QRegion &region)
{
    QPlatformBackingStore *store = backingStore();
    bool has_alpha = store->window()->property("ukui_xcb_TransparentBackground").toBool();

    if (!has_alpha)
        ukui_xcb_overridePaintDevice.setLocalData(true);

    VtableHook::callOriginalFun(store, &QPlatformBackingStore::beginPaint, region);

    ukui_xcb_overridePaintDevice.setLocalData(false);
}

void ukuiPlatformBackingStoreHelper::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    if (!backingStore()->paintDevice())
        return;
    return VtableHook::callOriginalFun(this->backingStore(), &QPlatformBackingStore::flush, window, region, offset);
}

#ifdef Q_OS_LINUX
void ukuiPlatformBackingStoreHelper::resize(const QSize &size, const QRegion &staticContents)
{

}
#endif

UKUI_END_NAMESPACE

