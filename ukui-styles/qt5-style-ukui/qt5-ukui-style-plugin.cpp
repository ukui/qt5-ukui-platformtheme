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

#include "qt5-ukui-style-plugin.h"
#include "qt5-ukui-style.h"
#include <private/qfusionstyle_p.h>

#include <QApplication>

#include <QDebug>

Qt5UKUIStylePlugin::Qt5UKUIStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
}

QStyle *Qt5UKUIStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new QFusionStyle;
    //FIXME:
    bool dark = false;
    if (key == "ukui-black") {
        qDebug()<<"use ukui-black";
        dark = true;
    } else {
        qDebug()<<"use ukui-white";
    }
    return new Qt5UKUIStyle(dark);
}

const QStringList Qt5UKUIStylePlugin::blackList()
{
    QStringList l;
    l<<"kylin-assistant";
    l<<"kylin-video";
    return l;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qt5-style-ukui, Qt5UKUIStylePlugin)
#endif // QT_VERSION < 0x050000
