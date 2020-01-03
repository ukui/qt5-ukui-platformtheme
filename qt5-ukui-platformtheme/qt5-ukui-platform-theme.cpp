#include <QVariant>
#include <QStandardPaths>
#include "qt5-ukui-platform-theme.h"
#include "ukui-style-settings.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QFileInfo>
#include <QIcon>
#endif

#include <QDebug>

Qt5UKUIPlatformTheme::Qt5UKUIPlatformTheme(const QStringList &args)
{
    //FIXME:
    Q_UNUSED(args)
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        qDebug()<<"set icon theme"<<settings->get("icon-theme-name").toString();
        //QIcon::setThemeName(settings->get("icon-theme-name").toString());
        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            qDebug()<<key<<"changed";
            if (key == "iconThemeName") {
                qDebug()<<"icon theme changed";
                QIcon::setThemeName(settings->get("icon-theme-name").toString());
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
    return QPlatformTheme::font(type);
}

QVariant Qt5UKUIPlatformTheme::themeHint(ThemeHint hint) const
{
    //FIXME:
    qDebug()<<"theme hint"<<hint;
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
