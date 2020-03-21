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

#include <QVariant>
#include <QStandardPaths>
#include "qt5-ukui-platform-theme.h"
#include "ukui-style-settings.h"

#include <QFontDatabase>
#include <QApplication>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QFileInfo>
#include <QIcon>
#endif

#include <QApplication>
#include <QWidget>

#include <QDebug>

Qt5UKUIPlatformTheme::Qt5UKUIPlatformTheme(const QStringList &args)
{
    //FIXME:
    Q_UNUSED(args)
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();

        //set font
        auto fontName = settings->get("systemFont").toString();
        auto fontSize = settings->get("systemFontSize").toInt();
        m_system_font.setFamily(fontName);
        m_system_font.setPixelSize(fontSize);

        m_fixed_font.setFamily(fontName);
        m_fixed_font.setPixelSize(fontSize*1.2);

        //QIcon::setThemeName(settings->get("icon-theme-name").toString());
        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            qDebug()<<key<<"changed";
            if (key == "iconThemeName") {
                qDebug()<<"icon theme changed";
                QIcon::setThemeName(settings->get("icon-theme-name").toString());
                // update all widgets for repaint new themed icons.
                for (auto widget : QApplication::allWidgets()) {
                    widget->update();
                }
            }
            if (key == "systemFont") {
                QString font = settings->get("system-font").toString();
                QFontDatabase db;
                if (db.families().contains(font)) {
                    QFont oldFont = QApplication::font();
                    m_system_font.setFamily(font);
                    m_fixed_font.setFamily(font);
                    oldFont.setFamily(font);
                    QApplication::setFont(oldFont);
                }
            }
            if (key == "systemFontSize") {
                int fontSize = settings->get("system-font-size").toInt();
                QFontDatabase db;
                if (fontSize > 0) {
                    QFont oldFont = QApplication::font();
                    m_system_font.setPixelSize(fontSize);
                    m_fixed_font.setPixelSize(fontSize*1.2);
                    oldFont.setPixelSize(fontSize);
                    QApplication::setFont(oldFont);
                }
            }
        });
    }
}

Qt5UKUIPlatformTheme::~Qt5UKUIPlatformTheme()
{
}

const QPalette *Qt5UKUIPlatformTheme::palette(Palette type) const
{
    //FIXME:
    return QPlatformTheme::palette(type);
}

const QFont *Qt5UKUIPlatformTheme::font(Font type) const
{
    //FIXME:
    return &m_system_font;
    switch (type) {
    case SystemFont:
        return &m_system_font;
    case TitleBarFont:
    case FixedFont:
    case GroupBoxTitleFont:
        return &m_fixed_font;
    default:
        return &m_system_font;
    }
    return QPlatformTheme::font(type);
}

QVariant Qt5UKUIPlatformTheme::themeHint(ThemeHint hint) const
{
    //FIXME:
    //qDebug()<<"theme hint"<<hint;
    switch (hint) {
    case QPlatformTheme::StyleNames:
        return QStringList()<<"ukui";
    case QPlatformTheme::SystemIconThemeName: {
        qDebug()<<"request icon theme name";
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            return UKUIStyleSettings::globalInstance()->get("icon-theme-name");
        }
        return "hicolor";
    }
    case QPlatformTheme::SystemIconFallbackThemeName:
        return "ukui-icon-theme-default";
    case QPlatformTheme::IconThemeSearchPaths:
        //FIXME:
        return QStringList()<<"/usr/share/icons";
    default:
        break;
    }
    return QPlatformTheme::themeHint(hint);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
bool Qt5UKUIPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    //FIXME:
    return false;
}

QPlatformDialogHelper *Qt5UKUIPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    return QPlatformTheme::createPlatformDialogHelper(type);
}
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
QIcon Qt5UKUIPlatformTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    //FIXME:
    return QPlatformTheme::fileIcon(fileInfo, iconOptions);
}
#endif

#ifdef GLOBAL_MENU
QPlatformMenuBar *Qt5UKUIPlatformTheme::createPlatformMenuBar() const
{
    return QPlatformTheme::createPlatformMenuBar();
}
#endif

#ifdef DBUS_TRAY
QPlatformSystemTrayIcon *Qt5UKUIPlatformTheme::createPlatformSystemTrayIcon() const
{
    return QPlatformTheme::createPlatformSystemTrayIcon();
}
#endif
