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

#include "black-list.h"
#include "ukui-style-settings.h"
#include "highlight-effect.h"

#include <QApplication>
#include <QTimer>

#include <QDebug>

Qt5UKUIStylePlugin::Qt5UKUIStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
//        QTimer::singleShot(1000, this, [=](){
            QString inconTheme = settings->get("iconThemeName").toString();
            if (inconTheme == "ukui-icon-theme-classical" || inconTheme == "ukui-classical") {
                HighLightEffect::setSymoblicColor(QColor(128, 128, 128, 255));
            }
//        });

        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "iconThemeName") {
                QString inconTheme = settings->get("iconThemeName").toString();
                if (inconTheme == "ukui-icon-theme-classical" || inconTheme == "ukui-classical") {
                    HighLightEffect::setSymoblicColor(QColor(128, 128, 128, 255));
                } else {
                    HighLightEffect::setSymoblicColor(QColor(31, 32, 34, 192));
                }
            }
        });
    }
}

QStyle *Qt5UKUIStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new Style;
    //FIXME:
    bool dark = false;
    bool useDefault = false;
    QString type = "default";

    if (key == "ukui-light") {
        dark = false;
        useDefault = false;
    } else if (key == "ukui-dark") {
        dark = true;
        useDefault = false;
    } else {
        useDefault = true;
    }

    type = "default";

    return new Qt5UKUIStyle(dark, useDefault, type);
}

const QStringList Qt5UKUIStylePlugin::blackList()
{
    return blackAppList();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(qt5-style-ukui, Qt5UKUIStylePlugin)
#endif // QT_VERSION < 0x050000
