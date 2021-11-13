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
#include "utility.h"

#include "utility.h"
#include "qxcbintegration.h"
#include "qxcbconnection.h"
#include "qxcbscreen.h"

#include "ukui-platform-integration.h"

#include <QPixmap>
#include <QPainter>
#include <QCursor>
#include <QDebug>
#include <QtX11Extras/QX11Info>
#include <QGuiApplication>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformcursor.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
#include <QtWidgets/qtwidgetsglobal.h>
#endif

#include <xcb/shape.h>
#include <xcb/xcb_icccm.h>

#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

#define XATOM_MOVE_RESIZE "_NET_WM_MOVERESIZE"
#define _GTK_SHOW_WINDOW_MENU "_GTK_SHOW_WINDOW_MENU"

QT_BEGIN_NAMESPACE
//extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

UKUI_BEGIN_NAMESPACE

QImage Utility::dropShadow(const QPixmap &px, qreal radius, const QColor &color)
{
    if (px.isNull())
        return QImage();

    QImage tmp(px.size() + QSize(radius * 2, radius * 2), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPoint(radius, radius), px);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmp, radius, false, true);
    blurPainter.end();

    if (color == QColor(Qt::black))
        return blurred;

    tmp = blurred;

    // blacken the image...
    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color);
    tmpPainter.end();

    return tmp;
}

QList<QRect> Utility::sudokuByRect(const QRect &rect, QMargins borders)
{
    QList<QRect> list;
    const QRect &contentsRect = rect - borders;

    list << QRect(0, 0, borders.left(), borders.top());
    list << QRect(list.at(0).topRight(), QSize(contentsRect.width(), borders.top())).translated(1, 0);
    list << QRect(list.at(1).topRight(), QSize(borders.right(), borders.top())).translated(1, 0);
    list << QRect(list.at(0).bottomLeft(), QSize(borders.left(), contentsRect.height())).translated(0, 1);
    list << contentsRect;
    list << QRect(contentsRect.topRight(), QSize(borders.right(), contentsRect.height())).translated(1, 0);
    list << QRect(list.at(3).bottomLeft(), QSize(borders.left(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomLeft(), QSize(contentsRect.width(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomRight(), QSize(borders.left(), borders.bottom())).translated(1, 1);

    return list;
}

QImage Utility::borderImage(const QPixmap &px, const QMargins &borders,
                            const QSize &size, QImage::Format format)
{
    QImage image(size, format);
    QPainter pa(&image);

    const QList<QRect> sudoku_src = sudokuByRect(px.rect(), borders);
    const QList<QRect> sudoku_tar = sudokuByRect(QRect(QPoint(0, 0), size), borders);

    pa.setCompositionMode(QPainter::CompositionMode_Source);

    for (int i = 0; i < 9; ++i) {
        pa.drawPixmap(sudoku_tar[i], px, sudoku_src[i]);
    }

    pa.end();

    return image;
}

xcb_atom_t Utility::internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

xcb_atom_t Utility::internAtom(const char *name, bool only_if_exists)
{
    return internAtom(QX11Info::connection(), name, only_if_exists);
}

void Utility::startWindowSystemMove(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_MOVE);
}

void Utility::cancelWindowMoveResize(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_CANCEL);
}

void Utility::showWindowSystemMenu(quint32 WId, QPoint globalPos)
{
    if (globalPos.isNull()) {
        globalPos = qApp->primaryScreen()->handle()->cursor()->pos();
    }

    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom(_GTK_SHOW_WINDOW_MENU);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[1] = globalPos.x();
    xev.data.data32[2] = globalPos.y();

    // ungrab鼠标，防止窗管的菜单窗口无法grab鼠标
    xcb_ungrab_pointer(QX11Info::connection(), XCB_CURRENT_TIME);
    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}

void Utility::setFrameExtents(WId wid, const QMargins &margins)
{
    xcb_atom_t frameExtents = internAtom("_GTK_FRAME_EXTENTS");

    if (frameExtents == XCB_NONE) {
        qWarning() << "Failed to create atom with name _GTK_FRAME_EXTENTS";
        return;
    }

    int32_t datas[4];
    datas[0] = int32_t(margins.left());
    datas[1] = int32_t(margins.right());
    datas[2] = int32_t(margins.top());
    datas[3] = int32_t(margins.bottom());

    xcb_change_property_checked(QX11Info::connection(), XCB_PROP_MODE_REPLACE, xcb_window_t(wid), frameExtents, XCB_ATOM_CARDINAL, 32, 4, datas);
}

static QVector<xcb_rectangle_t> qregion2XcbRectangles(const QRegion &region)
{
    QVector<xcb_rectangle_t> rectangles;

    rectangles.reserve(region.rectCount());

    for (const QRect &rect : region.rects()) {
        xcb_rectangle_t r;

        r.x = rect.x();
        r.y = rect.y();
        r.width = rect.width();
        r.height = rect.height();

        rectangles << r;
    }

    return rectangles;
}

static void setShapeRectangles(quint32 WId, const QVector<xcb_rectangle_t> &rectangles, bool onlyInput, bool transparentInput = false)
{
    xcb_shape_mask(QX11Info::connection(), XCB_SHAPE_SO_SET,
                   XCB_SHAPE_SK_BOUNDING, WId, 0, 0, XCB_NONE);

    if (transparentInput) {
        xcb_shape_rectangles(QX11Info::connection(), XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                             XCB_CLIP_ORDERING_YX_BANDED, WId, 0, 0, 0, 0);

        if (onlyInput)
            return;
    } else {
        xcb_shape_mask(QX11Info::connection(), XCB_SHAPE_SO_SET,
                       XCB_SHAPE_SK_INPUT, WId, 0, 0, XCB_NONE);
    }

    if (rectangles.isEmpty()) {
        return;
    }

    xcb_shape_rectangles(QX11Info::connection(), XCB_SHAPE_SO_SET, onlyInput ? XCB_SHAPE_SK_INPUT : XCB_SHAPE_SK_BOUNDING,
                         XCB_CLIP_ORDERING_YX_BANDED, WId, 0, 0, rectangles.size(), rectangles.constData());
}

void Utility::setShapeRectangles(quint32 WId, const QRegion &region, bool onlyInput, bool transparentInput)
{
    ::setShapeRectangles(WId, qregion2XcbRectangles(region), onlyInput, transparentInput);
}

void Utility::setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput, bool transparentInput)
{
    if (path.isEmpty()) {
        return ::setShapeRectangles(WId, QVector<xcb_rectangle_t>(), onlyInput, transparentInput);
    }

    QVector<xcb_rectangle_t> rectangles;

    foreach(const QPolygonF &polygon, path.toFillPolygons()) {
        foreach(const QRect &area, QRegion(polygon.toPolygon()).rects()) {
            xcb_rectangle_t rectangle;

            rectangle.x = area.x();
            rectangle.y = area.y();
            rectangle.width = area.width();
            rectangle.height = area.height();

            rectangles.append(std::move(rectangle));
        }
    }

    ::setShapeRectangles(WId, rectangles, onlyInput, transparentInput);
}


void Utility::sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos, Qt::MouseButton qbutton)
{
    int xbtn = qbutton == Qt::LeftButton ? XCB_BUTTON_INDEX_1 :
               qbutton == Qt::RightButton ? XCB_BUTTON_INDEX_3 :
               XCB_BUTTON_INDEX_ANY;

    if (globalPos.isNull()) {
        globalPos = qApp->primaryScreen()->handle()->cursor()->pos();
    }

    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom(XATOM_MOVE_RESIZE);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = action;
    xev.data.data32[3] = xbtn;
    xev.data.data32[4] = 0;

    if (action != _NET_WM_MOVERESIZE_CANCEL)
        xcb_ungrab_pointer(QX11Info::connection(), QX11Info::appTime());

    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}


QWindow *Utility::getWindowById(quint32 WId)
{
    for (QWindow *w : qApp->allWindows()) {
        if (w->handle() && w->handle()->winId() == WId) {
            return w;
        }
    }

    return Q_NULLPTR;
}

qreal Utility::getWindowDevicePixelRatio(quint32 WId)
{
    if (const QWindow *w = getWindowById(WId))
        return w->devicePixelRatio();

    return qApp->devicePixelRatio();
}

void Utility::startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos)
{
    sendMoveResizeMessage(WId, cornerEdge, globalPos);
}

static xcb_cursor_t CornerEdge2Xcb_cursor_t(Utility::CornerEdge ce)
{
    switch (ce) {
    case Utility::TopEdge:
        return XC_top_side;
    case Utility::TopRightCorner:
        return XC_top_right_corner;
    case Utility::RightEdge:
        return XC_right_side;
    case Utility::BottomRightCorner:
        return XC_bottom_right_corner;
    case Utility::BottomEdge:
        return XC_bottom_side;
    case Utility::BottomLeftCorner:
        return XC_bottom_left_corner;
    case Utility::LeftEdge:
        return XC_left_side;
    case Utility::TopLeftCorner:
        return XC_top_left_corner;
    default:
        return XCB_CURSOR_NONE;
    }
}

bool Utility::setWindowCursor(quint32 WId, Utility::CornerEdge ce)
{
    const auto display = QX11Info::display();

    Cursor cursor = XCreateFontCursor(display, CornerEdge2Xcb_cursor_t(ce));

    if (!cursor) {
        qWarning() << "[ui]::setWindowCursor() call XCreateFontCursor() failed";
        return false;
    }

    const int result = XDefineCursor(display, WId, cursor);

    XFlush(display);

    return result == Success;
}

QRegion Utility::regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset)
{
    QRegion tmp;

    for (const QRect &rect : region.rects()) {
        tmp += rect.translated(offset) + margins;
    }

    return tmp;
}

QByteArray Utility::windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len)
{
    QByteArray data;
    xcb_connection_t* conn = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, WId, propAtom, typeAtom, 0, len);
    xcb_generic_error_t* err = nullptr;
    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, &err);

    if (reply != nullptr) {
        len = xcb_get_property_value_length(reply);
        const char* buf = static_cast<const char*>(xcb_get_property_value(reply));
        data.append(buf, len);
        free(reply);
    }

    if (err != nullptr) {
        free(err);
    }

    return data;
}

void Utility::setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format)
{
    xcb_connection_t* conn = QX11Info::connection();
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, WId, propAtom, typeAtom, format, len, data);
    xcb_flush(conn);
}

void Utility::clearWindowProperty(quint32 WId, xcb_atom_t propAtom)
{
    xcb_delete_property_checked(QX11Info::connection(), WId, propAtom);
}

QPoint Utility::translateCoordinates(const QPoint &pos, quint32 src, quint32 dst)
{
    QPoint ret;
    xcb_translate_coordinates_cookie_t cookie =
        xcb_translate_coordinates(ukuiPlatformIntegration::xcbConnection()->xcb_connection(), src, dst,
                                  pos.x(), pos.y());
    xcb_translate_coordinates_reply_t *reply =
        xcb_translate_coordinates_reply(ukuiPlatformIntegration::xcbConnection()->xcb_connection(), cookie, NULL);
    if (reply) {
        ret.setX(reply->dst_x);
        ret.setY(reply->dst_y);
        free(reply);
    }

    return ret;
}

QRect Utility::windowGeometry(quint32 WId)
{
    xcb_get_geometry_reply_t *geom =
        xcb_get_geometry_reply(
            ukuiPlatformIntegration::xcbConnection()->xcb_connection(),
            xcb_get_geometry(ukuiPlatformIntegration::xcbConnection()->xcb_connection(), WId),
            NULL);

    QRect rect;

    if (geom) {
        // --
        // add the border_width for the window managers frame... some window managers
        // do not use a border_width of zero for their frames, and if we the left and
        // top strut, we ensure that pos() is absolutely correct.  frameGeometry()
        // will still be incorrect though... perhaps i should have foffset as well, to
        // indicate the frame offset (equal to the border_width on X).
        // - Brad
        // -- copied from qwidget_x11.cpp

        rect = QRect(geom->x, geom->y, geom->width, geom->height);

        free(geom);
    }

    return rect;
}

UKUI_END_NAMESPACE


QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const Utility::BlurArea &area)
{
    QDebugStateSaver saver(deg);
    Q_UNUSED(saver)

    deg.setAutoInsertSpaces(true);
    deg << "x:" << area.x
        << "y:" << area.y
        << "width:" << area.width
        << "height:" << area.height
        << "xRadius:" << area.xRadius
        << "yRadius:" << area.yRaduis;

    return deg;
}
QT_END_NAMESPACE
