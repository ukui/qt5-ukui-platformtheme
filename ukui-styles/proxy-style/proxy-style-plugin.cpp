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

#include "proxy-style-plugin.h"
#include "proxy-style.h"
#include "ukui-style-settings.h"

#include "application-style-settings.h"

#include <QApplication>
#include <QStyleFactory>

#include <QDebug>

using namespace UKUI;

ProxyStylePlugin::ProxyStylePlugin()
{
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        connect(settings, &UKUIStyleSettings::changed, this, [=](const QString &key){
            auto appStyleSettings = ApplicationStyleSettings::getInstance();
            if (appStyleSettings->currentStyleStretagy() != ApplicationStyleSettings::Default)
                return;

            if (key == "styleName") {
                qDebug()<<"style name changed";
                //We should not swich a application theme which use internal style.
                if (QApplication::style()->inherits("InternalStyle"))
                    return;

                auto styleName = settings->get("styleName").toString();

                if (styleName == "ukui") {
                    styleName = "ukui-white";
                }
                if (!QStyleFactory::keys().contains(styleName)) {
                    styleName = "fusion";
                }

                QApplication::setStyle(new ProxyStyle(styleName));
                QPalette palette = QApplication::palette();
                /*!
                  \todo implemet palette switch.
                  */
                switch (appStyleSettings->currentColorStretagy()) {
                case ApplicationStyleSettings::System: {
                    break;
                }
                case ApplicationStyleSettings::Bright: {
                    break;
                }
                case ApplicationStyleSettings::Dark: {
                    break;
                }
                default:
                    break;
                }
                QApplication::setPalette(palette);
            }

            if (key == "systemPalette") {
                onSystemPaletteChanged();
            }
            if (key == "useSystemPalette") {
                onSystemPaletteChanged();
            }
        });
    }
}

QStyle *ProxyStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new QProxyStyle("fusion");
    qDebug()<<"create"<<key;
    if (key == "ukui") {
        //FIXME:
        //get current style, fusion for invalid.
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            m_current_style_name = UKUIStyleSettings::globalInstance()->get("styleName").toString();
            return new ProxyStyle(UKUIStyleSettings::globalInstance()->get("styleName").toString());
        }
        qDebug()<<"ukui create proxy style";
        return new ProxyStyle(nullptr);
    }
    qDebug()<<"ukui create proxy style: null";
    return nullptr;
}

const QStringList ProxyStylePlugin::blackList()
{
    QStringList l;
    l<<"kylin-assistant";
    l<<"kylin-video";
    l<<"ukui-control-center";
    l<<"ubuntu-kylin-software-center";
    l<<"kylin-burner";
    return l;
}

void ProxyStylePlugin::onSystemPaletteChanged()
{
    bool useSystemPalette = UKUIStyleSettings::globalInstance()->get("useSystemPalette").toBool();
    if (useSystemPalette) {
        auto data = UKUIStyleSettings::globalInstance()->get("systemPalette");
        if (data.isNull())
            return;
        auto palette = qvariant_cast<QPalette>(data);
        QApplication::setPalette(palette);
    } else {
        auto palette = QApplication::style()->standardPalette();
        QApplication::setPalette(palette);
    }
}
