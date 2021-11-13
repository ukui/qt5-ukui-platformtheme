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
#ifndef UTILITY_H
#define UTILITY_H

#include <QImage>
#include <QPainterPath>

#include "global.h"

QT_BEGIN_NAMESPACE
class QXcbWindow;
QT_END_NAMESPACE

typedef uint32_t xcb_atom_t;
typedef struct xcb_connection_t xcb_connection_t;

UKUI_BEGIN_NAMESPACE

class Utility
{
public:
    enum CornerEdge {
        TopLeftCorner = 0,
        TopEdge = 1,
        TopRightCorner = 2,
        RightEdge = 3,
        BottomRightCorner = 4,
        BottomEdge = 5,
        BottomLeftCorner = 6,
        LeftEdge = 7
    };

    static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color);
    static QImage borderImage(const QPixmap &px, const QMargins &borders, const QSize &size,
                              QImage::Format format = QImage::Format_ARGB32_Premultiplied);

    static QList<QRect> sudokuByRect(const QRect &rect, QMargins borders);

    static xcb_atom_t internAtom(const char *name, bool only_if_exists = true);
    static xcb_atom_t internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists = true);
    static void startWindowSystemMove(quint32 WId);
    static void cancelWindowMoveResize(quint32 WId);

    static void showWindowSystemMenu(quint32 WId, QPoint globalPos = QPoint());
    static void setFrameExtents(WId wid, const QMargins &margins);
    static void setShapeRectangles(quint32 WId, const QRegion &region, bool onlyInput = true, bool transparentInput = false);
    static void setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput = true, bool transparentInput = false);
    static void startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos = QPoint());
    static bool setWindowCursor(quint32 WId, CornerEdge ce);

    static QRegion regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset = QPoint(0, 0));

    static QByteArray windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len);
    static void setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format = 8);
    static void clearWindowProperty(quint32 WId, xcb_atom_t propAtom);


    struct BlurArea {
        qint32 x;
        qint32 y;
        qint32 width;
        qint32 height;
        qint32 xRadius;
        qint32 yRaduis;

        inline BlurArea operator *(qreal scale)
        {
            if (qFuzzyCompare(1.0, scale))
                return *this;

            BlurArea new_area;

            new_area.x = qRound64(x * scale);
            new_area.y = qRound64(y * scale);
            new_area.width = qRound64(width * scale);
            new_area.height = qRound64(height * scale);
            new_area.xRadius = qRound64(xRadius * scale);
            new_area.yRaduis = qRound64(yRaduis * scale);

            return new_area;
        }

        inline BlurArea &operator *=(qreal scale)
        {
            return *this = *this * scale;
        }
    };

    struct QtMotifWmHints {
        quint32 flags, functions, decorations;
        qint32 input_mode;
        quint32 status;
    };

    static QPoint translateCoordinates(const QPoint &pos, quint32 src, quint32 dst);
    static QRect windowGeometry(quint32 WId);

private:
    static void sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos = QPoint(), Qt::MouseButton qbutton = Qt::LeftButton);
    static QWindow *getWindowById(quint32 WId);
    static qreal getWindowDevicePixelRatio(quint32 WId);
};

UKUI_END_NAMESPACE

QT_BEGIN_NAMESPACE

UKUI_USE_NAMESPACE

QDebug operator<<(QDebug deg, const Utility::BlurArea &area);
inline QPainterPath operator *(const QPainterPath &path, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return path;

    QPainterPath new_path = path;

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);

        new_path.setElementPositionAt(i, qRound(e.x * scale), qRound(e.y * scale));
    }

    return new_path;
}
inline QPainterPath &operator *=(QPainterPath &path, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return path;

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);

        path.setElementPositionAt(i, qRound(e.x * scale), qRound(e.y * scale));
    }

    return path;
}
inline QRect operator *(const QRect &rect, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return rect;

    return QRect(qRound(rect.left() * scale), qRound(rect.top() * scale),
                 qRound(rect.width() * scale), qRound(rect.height() * scale));
}
inline QMargins operator -(const QRect &r1, const QRect &r2)
{
    return QMargins(r2.left() - r1.left(), r2.top() - r1.top(),
                    r1.right() - r2.right(), r1.bottom() - r2.bottom());
}
inline QRegion operator *(const QRegion &pointRegion, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return pointRegion;

    QRegion pixelRegon;
    foreach (const QRect &rect, pointRegion.rects()) {
        pixelRegon += rect * scale;
    }
    return pixelRegon;
}
QT_END_NAMESPACE

#endif // UTILITY_H
