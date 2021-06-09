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

#include "ukui-platform-integration.h"
#include "global.h"
#include "vtablehook.h"

#ifdef Q_OS_LINUX
#define private public
#include "qxcbcursor.h"
#include "qxcbdrag.h"
#undef private

#include "ukui-xcb-xsettings.h"
#include "ukui-highdpi.h"
#include "xcb-native-event-filter.h"
#include "ukui-platform-native-interface-hook.h"

#include "qxcbscreen.h"
#include "qxcbbackingstore.h"

#include <X11/cursorfont.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_renderutil.h>
#endif

#include <private/qwidgetwindow_p.h>

#include <QWidget>
#include <QGuiApplication>
#include <QLibrary>
#include <QDrag>
#include <QStyleHints>

#include <private/qguiapplication_p.h>
#define protected public
#include <private/qsimpledrag_p.h>
#undef protected
#include <qpa/qplatformnativeinterface.h>
#include <private/qpaintengine_raster_p.h>

#ifdef Q_OS_LINUX
#define private public
#include "qxcbwindow.h"
#include "qxcbclipboard.h"
typedef QXcbScreen QNativeScreen;
typedef QXcbWindow QNativeWindow;
#elif defined(Q_OS_WIN)
typedef QWindowsScreen QNativeScreen;
typedef QWindowsWindow QNativeWindow;
#endif



class ukuiQPaintEngine : public QPaintEngine
{
public:
    inline void clearFeatures(const QPaintEngine::PaintEngineFeatures &f)
    {
        gccaps &= ~f;
    }
};

UKUI_BEGIN_NAMESPACE

ukuiXcbXSettings *ukuiPlatformIntegration::m_xsettings = nullptr;
ukuiPlatformIntegration::ukuiPlatformIntegration(const QStringList &parameters, int &argc, char **argv)
    : ukuiPlatformIntegrationParent(parameters, argc, argv)
{

}

ukuiPlatformIntegration::~ukuiPlatformIntegration()
{
#ifdef Q_OS_LINUX
    if (!m_eventFilter)
        return;

    qApp->removeNativeEventFilter(m_eventFilter);
    delete m_eventFilter;
#endif

#ifdef USE_NEW_IMPLEMENTING
    if (m_xsettings) {
        delete m_xsettings;
    }
#endif
}

QPlatformWindow *ukuiPlatformIntegration::createPlatformWindow(QWindow *window) const
{
    qDebug() << __FUNCTION__ << window << window->type() << window->parent();
    QPlatformWindow *w = ukuiPlatformIntegrationParent::createPlatformWindow(window);
    QNativeWindow *xw = static_cast<QNativeWindow*>(w);
    return xw;
}

QPlatformBackingStore *ukuiPlatformIntegration::createPlatformBackingStore(QWindow *window) const
{
    qDebug() << __FUNCTION__ << window << window->type() << window->parent();

    QPlatformBackingStore *store = ukuiPlatformIntegrationParent::createPlatformBackingStore(window);
    return store;
}

QPlatformOpenGLContext *ukuiPlatformIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QPlatformOpenGLContext *p_context = ukuiPlatformIntegrationParent::createPlatformOpenGLContext(context);

    return p_context;
}

QPaintEngine *ukuiPlatformIntegration::createImagePaintEngine(QPaintDevice *paintDevice) const
{
    static QPaintEngine::PaintEngineFeatures disable_features = QPaintEngine::PaintEngineFeatures(-1);

    QPaintEngine *base_engine = ukuiPlatformIntegrationParent::createImagePaintEngine(paintDevice);

    if (disable_features == 0)
        return base_engine;

    if (!base_engine)
        base_engine = new QRasterPaintEngine(paintDevice);

    ukuiQPaintEngine *engine = static_cast<ukuiQPaintEngine*>(base_engine);

    engine->clearFeatures(disable_features);

    return engine;
}

QStringList ukuiPlatformIntegration::themeNames() const
{
    QStringList list = ukuiPlatformIntegrationParent::themeNames();
    const QByteArray desktop_session = qgetenv("DESKTOP_SESSION");

    // 在lightdm环境中，无此环境变量
    if (desktop_session.isEmpty() || desktop_session == "ukui")
        list.prepend("ukui");

    return list;
}

#define XSETTINGS_CURSOR_BLINK QByteArrayLiteral("Net/CursorBlink")
#define XSETTINGS_CURSOR_BLINK_TIME QByteArrayLiteral("Net/CursorBlinkTime")
#define XSETTINGS_DOUBLE_CLICK_TIME QByteArrayLiteral("Net/DoubleClickTime")
#define GET_VALID_XSETTINGS(key) { \
    auto value = xSettings()->setting(key); \
    if (value.isValid()) return value; \
}

QVariant ukuiPlatformIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
#ifdef Q_OS_LINUX
    switch ((int)hint) {
    case CursorFlashTime:
        if (enableCursorBlink()) {
            GET_VALID_XSETTINGS(XSETTINGS_CURSOR_BLINK_TIME);
            break;
        } else {
            return 0;
        }
    case MouseDoubleClickInterval:
        GET_VALID_XSETTINGS(XSETTINGS_DOUBLE_CLICK_TIME);
        break;
    // QComboBox控件会根据此参数判断是应当在鼠标press还是release时弹出下拉列表
    // 此值默认为false，QFontComboBox在第一次弹出时有些卡顿，然而，ComboBox本身
    // 又会在收到Release事件时认为是对下拉列表进行了选中操作，卡顿会导致Release事件
    // 传递给了下拉列表，因此会出现第一次弹出列表后又立即隐藏的行为。将触发列表弹出的
    // 动作改为release则可避免此问题
    case SetFocusOnTouchRelease:
        return true;
    default:
        break;
    }
#endif

    return ukuiPlatformIntegrationParent::styleHint(hint);
}


QWindow *overrideTopLevelAt(QPlatformScreen *s, const QPoint &point)
{
    QWindow *window = static_cast<QNativeScreen*>(s)->QNativeScreen::topLevelAt(point);

    return window;
}

static void hookScreenGetWindow(QScreen *screen)
{
    if (screen && screen->handle())
        VtableHook::overrideVfptrFun(screen->handle(), &QPlatformScreen::topLevelAt, &overrideTopLevelAt);
}

static void watchScreenDPIChange(QScreen *screen)
{
    ukuiPlatformIntegration::instance()->xSettings()->registerCallbackForProperty("Qt/DPI/" + screen->name().toLocal8Bit(), &ukuiHighDpi::onDPIChanged, screen);
}


static void startDrag(QXcbDrag *drag)
{
    VtableHook::callOriginalFun(drag, &QXcbDrag::startDrag);

    QVector<xcb_atom_t> support_actions;
    const Qt::DropActions actions = drag->currentDrag()->supportedActions();

    if (actions.testFlag(Qt::CopyAction))
        support_actions << drag->atom(QXcbAtom::XdndActionCopy);

    if (actions.testFlag(Qt::MoveAction))
        support_actions << drag->atom(QXcbAtom::XdndActionMove);

    if (actions.testFlag(Qt::LinkAction))
        support_actions << drag->atom(QXcbAtom::XdndActionLink);

    if (support_actions.size() < 2)
        return;

    xcb_change_property(drag->xcb_connection(), XCB_PROP_MODE_REPLACE, drag->connection()->clipboard()->m_owner,
                        drag->atom(QXcbAtom::XdndActionList), XCB_ATOM_ATOM, sizeof(xcb_atom_t) * 8,
                        support_actions.size(), support_actions.constData());
    xcb_flush(drag->xcb_connection());
}


void ukuiPlatformIntegration::initialize()
{
    // 由于Qt很多代码中写死了是xcb，所以只能伪装成是xcb
    *QGuiApplicationPrivate::platform_name = "xcb";

    QXcbIntegration::initialize();
#ifdef Q_OS_LINUX
    m_eventFilter = new XcbNativeEventFilter(defaultConnection());
    qApp->installNativeEventFilter(m_eventFilter);

#endif

    for (QScreen *s : qApp->screens()) {
        hookScreenGetWindow(s);

        if (ukuiHighDpi::isActive()) {
            // 监听屏幕dpi变化
            watchScreenDPIChange(s);
        }
    }

    QObject::connect(qApp, &QGuiApplication::screenAdded, qApp, &hookScreenGetWindow);

    if (ukuiHighDpi::isActive()) {
        // 监听屏幕dpi变化
        QObject::connect(qApp, &QGuiApplication::screenAdded, qApp, &watchScreenDPIChange);
    }

}

#ifdef Q_OS_LINUX
static void onXSettingsChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    Q_UNUSED(connection)
    Q_UNUSED(property)
    Q_UNUSED(name)

    if (handle == reinterpret_cast<void*>(QPlatformIntegration::CursorFlashTime)) {
        // 由于QStyleHints中的属性值可能已经被自定义，因此不应该使用property中的值
        // 此处只表示 QStyleHints 的 cursorFlashTime 属性可能变了
        Q_EMIT qGuiApp->styleHints()->cursorFlashTimeChanged(qGuiApp->styleHints()->cursorFlashTime());
    }
}

bool ukuiPlatformIntegration::enableCursorBlink() const
{
    auto value = xSettings()->setting(XSETTINGS_CURSOR_BLINK);
    bool ok = false;
    int enable = value.toInt(&ok);

    return !ok || enable;
}


ukuiXcbXSettings *ukuiPlatformIntegration::xSettings(bool onlyExists) const
{
    if (onlyExists)
        return m_xsettings;

    return xSettings(xcbConnection());
}

ukuiXcbXSettings *ukuiPlatformIntegration::xSettings(QXcbConnection *connection)
{
    if (!m_xsettings) {
        auto xsettings = new ukuiXcbXSettings(connection->xcb_connection());
        m_xsettings = xsettings;

        // 注册回调，用于通知 QStyleHints 属性改变
        xsettings->registerCallbackForProperty(XSETTINGS_CURSOR_BLINK_TIME,
                                               onXSettingsChanged,
                                               reinterpret_cast<void*>(CursorFlashTime));
        xsettings->registerCallbackForProperty(XSETTINGS_CURSOR_BLINK,
                                               onXSettingsChanged,
                                               reinterpret_cast<void*>(CursorFlashTime));

        if (ukuiHighDpi::isActive()) {
            // 监听XSettings的dpi设置变化
            xsettings->registerCallbackForProperty("Xft/DPI", &ukuiHighDpi::onDPIChanged, nullptr);
        }
    }

    return m_xsettings;
}
#endif

UKUI_END_NAMESPACE
