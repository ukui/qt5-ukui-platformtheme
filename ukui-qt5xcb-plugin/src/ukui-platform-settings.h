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
#ifndef UKUIPLATFORMSETTINGS_H
#define UKUIPLATFORMSETTINGS_H

#include "global.h"

#include <QVariant>

QT_BEGIN_NAMESPACE
class QByteArray;
QT_END_NAMESPACE

UKUI_BEGIN_NAMESPACE

class ukuiPlatformSettings
{
public:
    virtual ~ukuiPlatformSettings() {}

    virtual bool initialized() const { return true; }
    virtual bool isEmpty() const { return false; }

    virtual bool contains(const QByteArray &property) const = 0;
    virtual QVariant setting(const QByteArray &property) const = 0;
    virtual void setSetting(const QByteArray &property, const QVariant &value) = 0;
    virtual QByteArrayList settingKeys() const = 0;

    virtual void emitSignal(const QByteArray &signal, qint32 data1, qint32 data2) = 0;

    typedef void (*PropertyChangeFunc)(const QByteArray &name, const QVariant &property, void *handle);
    void registerCallback(PropertyChangeFunc func, void *handle);
    void removeCallbackForHandle(void *handle);
    typedef void (*SignalFunc)(const QByteArray &signal, qint32 data1, qint32 data2, void *handle);
    void registerSignalCallback(SignalFunc func, void *handle);
    void removeSignalCallback(void *handle);

protected:
    void handlePropertyChanged(const QByteArray &property, const QVariant &value);
    void handleNotify(const QByteArray &signal, qint32 data1, qint32 data2);

    struct Q_DECL_HIDDEN Callback
    {
        PropertyChangeFunc func;
        void *handle;
    };

    struct Q_DECL_HIDDEN SignalCallback
    {
        SignalFunc func;
        void *handle;
    };

    std::vector<Callback> callback_links;
    std::vector<SignalCallback> signal_callback_links;
};

UKUI_END_NAMESPACE

#endif // UKUIPLATFORMSETTINGS_H
