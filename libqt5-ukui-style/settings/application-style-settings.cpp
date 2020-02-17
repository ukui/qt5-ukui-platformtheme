#include "application-style-settings.h"
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QStandardPaths>
#include <QStyle>

static ApplicationStyleSettings *global_instance = nullptr;

ApplicationStyleSettings *ApplicationStyleSettings::getInstance()
{
    if (!global_instance)
        global_instance = new ApplicationStyleSettings;
    return global_instance;
}

const QString ApplicationStyleSettings::currentCustomStyleName()
{
    if (m_style_stretagy == Default)
        return nullptr;
    return m_current_custom_style_name;
}

void ApplicationStyleSettings::setColorStretagy(ApplicationStyleSettings::ColorStretagy stretagy)
{
    if (m_color_stretagy != stretagy) {
        m_color_stretagy = stretagy;
        setValue("color-stretagy", stretagy);
        Q_EMIT colorStretageChanged(stretagy);
        QtConcurrent::run([=](){
            this->sync();
        });
    }
}

void ApplicationStyleSettings::setStyleStretagy(ApplicationStyleSettings::StyleStretagy stretagy)
{
    if (m_style_stretagy != stretagy) {
        m_style_stretagy = stretagy;
        setValue("style-stretagy", stretagy);
        Q_EMIT styleStretageChanged(stretagy);
        QtConcurrent::run([=](){
            this->sync();
        });
    }
}

void ApplicationStyleSettings::setCustomStyle(const QString &style)
{
    m_current_custom_style_name = style;
    QApplication::setStyle(style);
}


ApplicationStyleSettings::ApplicationStyleSettings(QObject *parent) : QSettings(parent)
{
    /*!
      \todo make settings together into an ini file.
      */
//    QString configFileName = QApplication::organizationDomain() + "." + QApplication::organizationName() + "." + QApplication::applicationName();
//    QString configDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/" + "ukui";
//    QString configPath = configDir + "/" + configFileName;
//    setPath(QSettings::IniFormat, QSettings::UserScope, configPath);
//    setDefaultFormat(QSettings::IniFormat);
    m_color_stretagy = ColorStretagy(value("color-stretagy").toInt());
    m_style_stretagy = StyleStretagy(value("style-stretagy").toInt());
    m_current_custom_style_name = value("custom-style").toString();
}
