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
#ifndef UKUIFOREIGNPLATFORMWINDOW_H
#define UKUIFOREIGNPLATFORMWINDOW_H

#include "global.h"

#include <QtGlobal>

#ifdef Q_OS_LINUX
#define private public
#include "qxcbwindow.h"
typedef QXcbWindow QNativeWindow;
#undef private
#elif defined(Q_OS_WIN)
#include "qwindowswindow.h"
typedef QWindowsWindow QNativeWindow;
#endif

UKUI_BEGIN_NAMESPACE


class ukuiForeignPlatformWindow : public QNativeWindow
{
public:
    explicit ukuiForeignPlatformWindow(QWindow *window, WId winId);
    ~ukuiForeignPlatformWindow();

    QRect geometry() const Q_DECL_OVERRIDE;
    QMargins frameMargins() const override;

#ifdef Q_OS_LINUX
    void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *) override;
    void handlePropertyNotifyEvent(const xcb_property_notify_event_t *) override;

    QNativeWindow *toWindow() override;
#endif

private:
    void create() override;
    void destroy() override;


    bool isForeignWindow() const override {
        return true;
    }
    void updateTitle();
    void updateWmClass();
    void updateWindowState();
    void updateWindowTypes();
    void updateProcessId();

    void init();

};

UKUI_END_NAMESPACE

#endif // UKUIFOREIGNPLATFORMWINDOW_H
