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
#include "ukui-highdpi.h"
#include "vtablehook.h"
#include "ukui-platform-integration.h"
#include "ukui-xcb-xsettings.h"
#include "qxcbscreen.h"
#include "qxcbwindow.h"
#include "qxcbbackingstore.h"

#include <private/qhighdpiscaling_p.h>
#include <private/qguiapplication_p.h>
#include <QGuiApplication>
#include <QDebug>

UKUI_BEGIN_NAMESPACE

bool ukuiHighDpi::active = false;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QHash<QPlatformScreen*, qreal> ukuiHighDpi::screenFactorMap;
#endif
QPointF ukuiHighDpi::fromNativePixels(const QPointF &pixelPoint, const QWindow *window)
{
    return QHighDpi::fromNativePixels(pixelPoint, window);
}

__attribute__((constructor)) // 在库被加载时就执行此函数
static void init()
{
    ukuiHighDpi::init();
}

void ukuiHighDpi::init()
{
    if (QGuiApplication::testAttribute(Qt::AA_DisableHighDpiScaling)
            // 无有效的xsettings时禁用
            || !ukuiXcbXSettings::getOwner()
            || (qEnvironmentVariableIsSet("QT_SCALE_FACTOR_ROUNDING_POLICY")
                && qgetenv("QT_SCALE_FACTOR_ROUNDING_POLICY") != "PassThrough")) {
        return;
    }

    // 禁用platform theme中的缩放机制
    qputenv("D_DISABLE_RT_SCREEN_SCALE", "1");
    // 设置为完全控制缩放比例，避免被Qt“4舍5入”了缩放比
    qputenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough");

    // 强制开启使用UXCB的缩放机制，此时移除会影响此功能的所有Qt环境变量
    if (qEnvironmentVariableIsSet("UKUI_XCB_FORCE_OVERRIDE_HIDPI")) {
        qunsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
        qunsetenv("QT_SCALE_FACTOR");
        qunsetenv("QT_SCREEN_SCALE_FACTORS");
        qunsetenv("QT_ENABLE_HIGHDPI_SCALING");
        qunsetenv("QT_USE_PHYSICAL_DPI");
    }

    if (!QGuiApplication::testAttribute(Qt::AA_EnableHighDpiScaling)) {
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QHighDpiScaling::initHighDpiScaling();
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    active = VtableHook::overrideVfptrFun(&QXcbScreen::pixelDensity, pixelDensity);

    if (active) {
        VtableHook::overrideVfptrFun(&QXcbScreen::logicalDpi, logicalDpi);
    }
#else
    active = VtableHook::overrideVfptrFun(&QXcbScreen::logicalDpi, logicalDpi);
#endif
}


bool ukuiHighDpi::isActive()
{
    return active;
}

bool ukuiHighDpi::overrideBackingStore()
{
    // 默认不开启，会降低绘图效率
    static bool enabled = qEnvironmentVariableIsSet("U_XCB_HIDPI_BACKINGSTORE");
    return enabled;
}


QDpi ukuiHighDpi::logicalDpi(QXcbScreen *s)
{
    static bool dpi_env_set = qEnvironmentVariableIsSet("QT_FONT_DPI");
    // 遵循环境变量的设置
    if (dpi_env_set) {
        return s->QXcbScreen::logicalDpi();
    }

    int dpi = 0;
    QVariant value = ukuiPlatformIntegration::xSettings(s->connection())->setting("Qt/DPI/" + s->name().toLocal8Bit());
    bool ok = false;

    dpi = value.toInt(&ok);

    // fallback
    if (!ok) {
        value = ukuiPlatformIntegration::xSettings(s->connection())->setting("Xft/DPI");
        dpi = value.toInt(&ok);
    }

    // fallback
    if (!ok) {
        return s->QXcbScreen::logicalDpi();
    }

    qreal d = dpi / 1024.0;

    return QDpi(d, d);
}


#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
qreal ukuiHighDpi::pixelDensity(QXcbScreen *s)
{
    qreal scale = screenFactorMap.value(s, 0);

    if (qIsNull(scale)) {
        scale = s->logicalDpi().first / 96.0;
        screenFactorMap[s] = scale;
    }

    return scale;
}
#endif

qreal ukuiHighDpi::devicePixelRatio(QPlatformWindow *w)
{
    qreal base_factor = QHighDpiScaling::factor(w->screen());
    return qCeil(base_factor) / base_factor;
}
#include <syslog.h>
void ukuiHighDpi::onDPIChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    static bool dynamic_dpi = qEnvironmentVariableIsSet("U_XCB_RT_HIDPI");
    syslog(LOG_ERR,"%s ========dynamic_dpi = %d",__func__, dynamic_dpi);
    if (!dynamic_dpi)
        return;

    Q_UNUSED(connection)
    Q_UNUSED(name)

    // 判断值是否有效
    if (!property.isValid())
        return;

    qInfo() << Q_FUNC_INFO << name << property;

    // 清理过期的屏幕缩放值
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if (QScreen *screen = reinterpret_cast<QScreen*>(handle)) {
        screenFactorMap.remove(screen->handle());
    } else {
        screenFactorMap.clear();
#else
    {
#endif
        // 刷新所有窗口
        for (QWindow *window : qGuiApp->allWindows()) {
            if (window->type() == Qt::Desktop)
                continue;

            // 更新窗口大小
            if (window->handle()) {
                QWindowSystemInterfacePrivate::GeometryChangeEvent gce(window, QHighDpi::fromNativePixels(window->handle()->geometry(), window));
                QGuiApplicationPrivate::processGeometryChangeEvent(&gce);
            }
        }
    }
}

UKUI_END_NAMESPACE
