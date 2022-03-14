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
#include "highlight-effect.h"

#include <QFontDatabase>
#include <QApplication>
#include <QTimer>

#include <QPluginLoader>
#include <QIconEnginePlugin>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QFileInfo>
#include <QIcon>
#endif

#include <QApplication>
#include <QWidget>

#include <QDebug>
#include <private/qgenericunixthemes_p.h>

#include <QQuickStyle>

#include "widget/message-box.h"


Qt5UKUIPlatformTheme::Qt5UKUIPlatformTheme(const QStringList &args)
{
    //FIXME:
    Q_UNUSED(args)
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();

        //set font
        auto fontName = settings->get("systemFont").toString();
        auto fontSize = settings->get("systemFontSize").toString().toDouble();
        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            fontSize = 11;
        m_system_font.setFamily(fontName);
        m_system_font.setPointSizeF(fontSize);

        m_fixed_font.setFamily(fontName);
        m_fixed_font.setPointSizeF(fontSize*1.2);

        /*!
         * \bug
         * if we set app font, qwizard title's font will
         * become very small. I handle the wizard title
         * in ProxyStyle::polish().
         */
        QApplication::setFont(m_system_font);

        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "iconThemeName") {
                QString icontheme = settings->get("icon-theme-name").toString();

                QIcon::setThemeName(icontheme);

                QIcon icon = qApp->windowIcon();
                qApp->setWindowIcon(QIcon::fromTheme(icon.name()));
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
                if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
                    return;
                double fontSize = settings->get("system-font-size").toString().toDouble();
                if (fontSize > 0) {
                    QFont oldFont = QApplication::font();
                    m_system_font.setPointSize(fontSize);
                    m_fixed_font.setPointSize(fontSize*1.2);
                    oldFont.setPointSizeF(fontSize);
                    QApplication::setFont(oldFont);
                }
            }
        });
    }

    // add qqc2 style
    if (QFile::exists(QString("%1/kf5/kirigami/org.kylin.style.so").arg(QT_PLUGIN_INSTALL_DIRS))) {
        QQuickStyle::setStyle("org.kylin.style");
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
    if (type == FixedFont)
        return &m_fixed_font;
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
    switch (hint) {
    case QPlatformTheme::StyleNames:
        return QStringList()<<"ukui";

    case QPlatformTheme::SystemIconThemeName: {
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            if (auto settings = UKUIStyleSettings::globalInstance()) {
                QString icontheme = settings->get("icon-theme-name").toString();

                return QStringList()<<icontheme;
            }
        }
        return "hicolor";
    }

    case QPlatformTheme::SystemIconFallbackThemeName:
        return "hicolor";
    case QPlatformTheme::IconThemeSearchPaths:
        //FIXME:
        return QStringList()<<".local/share/icons"<<"/usr/share/icons"<<"/usr/local/share/icons";
    default:
        break;
    }
    return QPlatformTheme::themeHint(hint);
}

QIconEngine *Qt5UKUIPlatformTheme::createIconEngine(const QString &iconName) const
{
//    QPluginLoader l(XDG_ICON_ENGINE_PATH);
//    if (l.instance()) {
//        auto p = dynamic_cast<QIconEnginePlugin *>(l.instance());
//        auto engine = p->create();
//        qDebug()<<"use my engine";
//        return engine;
//    } else {
//        qDebug()<<"use common engine";
//        return QPlatformTheme::createIconEngine(iconName);
//    }

//    //return new XdgIconLoaderEngine(iconName);

    return QPlatformTheme::createIconEngine(iconName);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
bool Qt5UKUIPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
    return true;
    switch (type) {
    case QPlatformTheme::FileDialog:
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return false;
    case QPlatformTheme::MessageDialog:
        if (qAppName() == "ukui-control-center" || qAppName() == "kybackup")
            return false;
        return true;
    default:
        break;
    }

    return false;
}

QPlatformDialogHelper *Qt5UKUIPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
    switch (type) {
    case QPlatformTheme::FileDialog:
    case QPlatformTheme::FontDialog:
    case QPlatformTheme::ColorDialog:
        return QPlatformTheme::createPlatformDialogHelper(type);
    case QPlatformTheme::MessageDialog:
        if (qAppName() == "kybackup")
            return nullptr;
        return new MessageBoxHelper;
    default:
        break;
    }

    return nullptr;
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
    QGnomeTheme *gnomeTheme = new QGnomeTheme();
    return gnomeTheme->createPlatformSystemTrayIcon();
}
#endif
