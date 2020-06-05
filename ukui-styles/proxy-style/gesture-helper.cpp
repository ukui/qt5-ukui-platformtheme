/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "gesture-helper.h"

#include <QWidget>
#include <QGestureEvent>
#include <QTapAndHoldGesture>

#include <QContextMenuEvent>

#include <QApplication>
#include <QMenu>

GestureHelper::GestureHelper(QObject *parent) : QObject(parent)
{
    m_timer.setInterval(500);
    m_timer.setSingleShot(true);
}

void GestureHelper::registerWidget(QWidget *widget)
{
    if (!widget)
        return;

    widget->removeEventFilter(this);

    widget->grabGesture(Qt::TapGesture);
    widget->grabGesture(Qt::TapAndHoldGesture);
    widget->grabGesture(Qt::PinchGesture);
    widget->grabGesture(Qt::PinchGesture);
    widget->grabGesture(Qt::SwipeGesture);

    widget->installEventFilter(this);
}

void GestureHelper::unregisterWidget(QWidget *widget)
{
    if (!widget)
        return;

    widget->removeEventFilter(this);

    widget->ungrabGesture(Qt::TapGesture);
    widget->ungrabGesture(Qt::TapAndHoldGesture);
    widget->ungrabGesture(Qt::PinchGesture);
    widget->ungrabGesture(Qt::PinchGesture);
    widget->ungrabGesture(Qt::SwipeGesture);
}

bool GestureHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        auto e = static_cast<QGestureEvent *>(event);
        auto widget = qobject_cast<QWidget *>(watched);
        if (!widget->isActiveWindow())
            return false;

        if (m_timer.isActive())
            return false;
        if (auto hg = static_cast<QTapAndHoldGesture*>(e->gesture(Qt::TapAndHoldGesture))) {
            switch (hg->state()) {
            case Qt::GestureStarted: {
                if (menu_popped) {
                    return false;
                } else {
                    menu_popped = true;
                    m_timer.start();
                    auto pos = widget->mapFromGlobal(hg->position().toPoint());
                    auto gpos = hg->position().toPoint();
                    QContextMenuEvent ce(QContextMenuEvent::Other, pos, gpos, Qt::NoModifier);
                    qApp->sendEvent(widget, &ce);
                }
                break;
            }
            case Qt::GestureCanceled:
            case Qt::GestureFinished: {
                menu_popped = false;
                break;
            }
            default:
                break;
            }
        }
    }
    return false;
}
