#ifndef QT5UKUIPLATFORMTHEME_H
#define QT5UKUIPLATFORMTHEME_H

#include "qt5-ukui-platformtheme_global.h"
#include <QObject>
#include <qpa/qplatformtheme.h>

#if !defined(QT_NO_DBUS) && defined(QT_DBUS_LIB)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)) && !defined(QT_NO_SYSTEMTRAYICON)
#define DBUS_TRAY
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
#define GLOBAL_MENU
#endif

#endif

class QPalette;
#ifdef DBUS_TRAY
class QPlatformSystemTrayIcon;
#endif

#ifdef GLOBAL_MENU
class QPlatformMenuBar;
#endif

/*!
 * \brief The Qt5UKUIPlatformTheme class
 * \details
 * In UKUI desktop environment, we have our own platform to manage the qt applications' style.
 * This class is used to take over the theme and preferences of those applications.
 * The platform theme will effect globally.
 */
class QT5UKUIPLATFORMTHEMESHARED_EXPORT Qt5UKUIPlatformTheme : public QObject, public QPlatformTheme
{
    Q_OBJECT
public:
    Qt5UKUIPlatformTheme(const QStringList &args);
    ~Qt5UKUIPlatformTheme();

    virtual const QPalette *palette(Palette type = SystemPalette) const;
    virtual const QFont *font(Font type = SystemFont) const;
    virtual QVariant themeHint(ThemeHint hint) const;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    virtual bool usePlatformNativeDialog(DialogType type) const;
    virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    virtual QIcon fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions = 0) const;
#endif

#ifdef GLOBAL_MENU
    virtual QPlatformMenuBar* createPlatformMenuBar() const;
#endif

#ifdef DBUS_TRAY
    virtual QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const;
#endif

};

#endif // QT5UKUIPLATFORMTHEME_H
