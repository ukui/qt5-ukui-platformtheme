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

#include <QGuiApplication>
#include "xcb-native-event-filter.h"

#define private public
#include "qxcbconnection.h"
#include "qxcbclipboard.h"
#undef private

#include "ukui-platform-integration.h"
//#include "ukui-xcb-wm-support.h"
#include "ukui-xcb-xsettings.h"

#include <xcb/xfixes.h>
#include <xcb/damage.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>

#include <cmath>

UKUI_BEGIN_NAMESPACE

XcbNativeEventFilter::XcbNativeEventFilter(QXcbConnection *connection)
    : m_connection(connection)
    , lastXIEventDeviceInfo(0, XIDeviceInfos())
{
    // init damage first event value
    xcb_prefetch_extension_data(connection->xcb_connection(), &xcb_damage_id);
    const auto* reply = xcb_get_extension_data(connection->xcb_connection(), &xcb_damage_id);

    if (reply->present) {
      m_damageFirstEvent = reply->first_event;
      xcb_damage_query_version_unchecked(connection->xcb_connection(), XCB_DAMAGE_MAJOR_VERSION, XCB_DAMAGE_MINOR_VERSION);
    } else {
        m_damageFirstEvent = 0;
    }

    updateXIDeviceInfoMap();
}


QClipboard::Mode XcbNativeEventFilter::clipboardModeForAtom(xcb_atom_t a) const
{
    if (a == XCB_ATOM_PRIMARY)
        return QClipboard::Selection;
    if (a == m_connection->atom(QXcbAtom::CLIPBOARD))
        return QClipboard::Clipboard;
    // not supported enum value, used to detect errors
    return QClipboard::FindBuffer;
}

typedef struct qt_xcb_ge_event_t {
    uint8_t  response_type;
    uint8_t  extension;
    uint16_t sequence;
    uint32_t length;
    uint16_t event_type;
} qt_xcb_ge_event_t;

static inline bool isXIEvent(xcb_generic_event_t *event, int opCode)
{
    qt_xcb_ge_event_t *e = (qt_xcb_ge_event_t *)event;
    return e->extension == opCode;
}

bool XcbNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    uint response_type = event->response_type & ~0x80;
    // cannot use isXFixesType because symbols from QXcbBasicConnection are not exported
    if (response_type == m_connection->m_xfixesFirstEvent + XCB_XFIXES_SELECTION_NOTIFY) {
        xcb_xfixes_selection_notify_event_t *xsn = (xcb_xfixes_selection_notify_event_t *)event;

        if (xsn->selection == ukuiPlatformIntegration::xcbConnection()->atom(QXcbAtom::_NET_WM_CM_S0)) {
            //ukuiXcbWMSupport::instance()->updateHasComposite();
        }

        QClipboard::Mode mode = clipboardModeForAtom(xsn->selection);
        if (mode > QClipboard::Selection)
            return false;

        // here we care only about the xfixes events that come from non Qt processes
        if (xsn->owner == XCB_NONE && xsn->subtype == XCB_XFIXES_SELECTION_EVENT_SET_SELECTION_OWNER) {
            QXcbClipboard *xcbClipboard = m_connection->m_clipboard;
            xcbClipboard->emitChanged(mode);
        }
    } else {
        switch (response_type) {
            // 修复Qt程序对触摸板的自然滚动开关后不能实时生效
            // 由于在收到xi的DeviceChanged事件后，Qt更新ScrollingDevice时没有更新verticalIncrement字段
            // 导致那些使用increment的正负值控制自然滚动开关的设备对Qt程序无法实时生效
            // 有些电脑上触摸板没有此问题，是因为他的系统环境中没有安装xserver-xorg-input-synaptics
#ifdef XCB_USE_XINPUT21
        case XCB_GE_GENERIC: {
            QXcbConnection *xcb_connect = ukuiPlatformIntegration::xcbConnection();

            if (xcb_connect->m_xi2Enabled && isXIEvent(event, xcb_connect->m_xiOpCode)) {
                xXIGenericDeviceEvent *xiEvent = reinterpret_cast<xXIGenericDeviceEvent *>(event);

                {
                    xXIDeviceEvent *xiDEvent = reinterpret_cast<xXIDeviceEvent*>(event);
                    // NOTE(zccrs): 获取设备编号，至于为何会偏移4个字节，参考：
                    // void QXcbConnection::xi2PrepareXIGenericDeviceEvent(xcb_ge_event_t *event)
                    // xcb event structs contain stuff that wasn't on the wire, the full_sequence field
                    // adds an extra 4 bytes and generic events cookie data is on the wire right after the standard 32 bytes.
                    // Move this data back to have the same layout in memory as it was on the wire
                    // and allow casting, overwriting the full_sequence field.
                    uint16_t source_id = *(&xiDEvent->sourceid + 2);

                    auto device = xiDeviceInfoMap.find(source_id);

                    // find device
                    if (device != xiDeviceInfoMap.constEnd()) {
                        lastXIEventDeviceInfo = qMakePair(xiDEvent->time, device.value());
                    }
                }

                if (xiEvent->evtype != XI_DeviceChanged) {
                    if (xiEvent->evtype == XI_HierarchyChanged) {
                        xXIHierarchyEvent *xiEvent = reinterpret_cast<xXIHierarchyEvent *>(event);
                        // We only care about hotplugged devices
                        if (!(xiEvent->flags & (XISlaveRemoved | XISlaveAdded)))
                            return false;

                        updateXIDeviceInfoMap();
                    }

                    return false;
                }

            }
            break;
        }
#endif
        case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t *ev = reinterpret_cast<xcb_client_message_event_t*>(event);

            if (ukuiXcbXSettings::handleClientMessageEvent(ev)) {
                return true;
            }
            break;
        }
        default:
            static auto updateScaleLogcailDpi = qApp->property("ukui_updateScaleLogcailDpi").toULongLong();
            // cannot use isXRandrType because symbols from QXcbBasicConnection are not exported
            if (updateScaleLogcailDpi && ukuiPlatformIntegration::xcbConnection()->hasXRender()
                    && response_type == ukuiPlatformIntegration::xcbConnection()->m_xrandrFirstEvent + XCB_RANDR_NOTIFY) {
                xcb_randr_notify_event_t *e = reinterpret_cast<xcb_randr_notify_event_t *>(event);
                xcb_randr_output_change_t output = e->u.oc;

                if (e->subCode == XCB_RANDR_NOTIFY_OUTPUT_CHANGE) {
                    QXcbScreen *screen = ukuiPlatformIntegration::xcbConnection()->findScreenForOutput(output.window, output.output);

                    if (!screen && output.connection == XCB_RANDR_CONNECTION_CONNECTED
                            && output.crtc != XCB_NONE && output.mode != XCB_NONE) {
                        ukuiPlatformIntegration::xcbConnection()->updateScreens(e);
                        // 通知ukui platform插件重设缩放后的dpi值
                        reinterpret_cast<void(*)()>(updateScaleLogcailDpi)();

                        return true;
                    }
                }
            }
            break;
        }
    }
    return false;
}

DeviceType XcbNativeEventFilter::xiEventSource(const QInputEvent *event) const
{
    if (lastXIEventDeviceInfo.first == event->timestamp())
        return lastXIEventDeviceInfo.second.type;

    return UnknowDevice;
}

void XcbNativeEventFilter::updateXIDeviceInfoMap()
{
    xiDeviceInfoMap.clear();

    QXcbConnection *xcb_connect = ukuiPlatformIntegration::xcbConnection();

    Display *xDisplay = reinterpret_cast<Display *>(xcb_connect->xlib_display());

    int deviceCount = 0;
    XIDeviceInfo *devices = XIQueryDevice(xDisplay, XIAllDevices, &deviceCount);

    for (int i = 0; i < deviceCount; ++i) {
        // Only non-master pointing devices are relevant here.
        if (devices[i].use != XISlavePointer)
            continue;

        int nprops;
        Atom *props = XIListProperties(xDisplay, devices[i].deviceid, &nprops);

        if (!nprops)
            return;

        char *name;

        for (int j = 0; j < nprops; ++j) {
            name = XGetAtomName(xDisplay, props[j]);

            if (QByteArrayLiteral("Synaptics Off") == name
                    || QByteArrayLiteral("libinput Tapping Enabled") == name) {
                xiDeviceInfoMap[devices[i].deviceid] = XIDeviceInfos(TouchapdDevice);
            } else if (QByteArrayLiteral("Button Labels") == name
                       || QByteArrayLiteral("libinput Button Scrolling Button") == name) {
                xiDeviceInfoMap[devices[i].deviceid] = XIDeviceInfos(MouseDevice);
            }

            XFree(name);
        }

        XFree(props);
    }

    // XIQueryDevice may return NULL..boom
    if (devices)
        XIFreeDeviceInfo(devices);
}

UKUI_END_NAMESPACE
