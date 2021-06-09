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
#ifndef UKUIXCBXSETTINGS_H
#define UKUIXCBXSETTINGS_H

#include "ukui-platform-settings.h"

#include <QByteArray>
#include <QByteArrayList>

#include <xcb/xcb.h>

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

UKUI_BEGIN_NAMESPACE

class ukuiXcbXSettingsPrivate;

class ukuiXcbXSettings : public ukuiPlatformSettings
{
    Q_DECLARE_PRIVATE(ukuiXcbXSettings)
public:
    ukuiXcbXSettings(xcb_connection_t *connection, const QByteArray &property = QByteArray());
    ukuiXcbXSettings(xcb_connection_t *connection, xcb_window_t setting_window, const QByteArray &property = QByteArray());
    ukuiXcbXSettings(xcb_window_t setting_window, const QByteArray &property = QByteArray());
    ~ukuiXcbXSettings();

    static xcb_window_t getOwner(xcb_connection_t *conn = nullptr, int screenNumber = 0);
    bool initialized() const override;
    bool isEmpty() const override;

    bool contains(const QByteArray &property) const override;
    QVariant setting(const QByteArray &property) const override;
    void setSetting(const QByteArray &property, const QVariant &value) override;
    QByteArrayList settingKeys() const override;

    typedef void (*PropertyChangeFunc)(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle);
    void registerCallback(PropertyChangeFunc func, void *handle);
    void registerCallbackForProperty(const QByteArray &property, PropertyChangeFunc func, void *handle);
    void removeCallbackForHandle(const QByteArray &property, void *handle);
    void removeCallbackForHandle(void *handle);
    typedef void (*SignalFunc)(xcb_connection_t *connection, const QByteArray &signal, qint32 data1, qint32 data2, void *handle);
    void registerSignalCallback(SignalFunc func, void *handle);
    void removeSignalCallback(void *handle);
    void emitSignal(const QByteArray &signal, qint32 data1, qint32 data2) override;

    static void emitSignal(xcb_connection_t *conn, xcb_window_t window, xcb_atom_t type, const QByteArray &signal, qint32 data1, qint32 data2);
    static bool handlePropertyNotifyEvent(const xcb_property_notify_event_t *event);
    static bool handleClientMessageEvent(const xcb_client_message_event_t *event);

    static void clearSettings(xcb_window_t setting_window);
private:
    ukuiXcbXSettingsPrivate *d_ptr;
    friend class ukuiXcbXSettingsPrivate;
};

UKUI_END_NAMESPACE

#endif // UKUIXCBXSETTINGS_H
