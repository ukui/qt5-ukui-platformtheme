#include <qpa/qplatformthemeplugin.h>
#include "qt5-ukui-platform-theme.h"

#include <QDebug>

#define UKUI_PLATFORMTHEME

QT_BEGIN_NAMESPACE

class Qt5UKUIPlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "ukui.json")

public:
    virtual QPlatformTheme *create(const QString &key, const QStringList &params) {
        qDebug()<<"platform ukui"<<key<<params;
        if (key.toLower() == "ukui") {
            qDebug()<<"platform ukui";
            return new Qt5UKUIPlatformTheme(params);
        }
        return nullptr;
    }
};

QT_END_NAMESPACE

#include "main.moc"
