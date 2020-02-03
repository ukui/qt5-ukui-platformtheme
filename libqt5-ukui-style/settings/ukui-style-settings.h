#ifndef UKUISTYLESETTINGS_H
#define UKUISTYLESETTINGS_H

#include "libqt5-ukui-style_global.h"
#include <QGSettings>

/*!
 * \brief The UKUIStyleSettings class
 * \details
 * To distingust with other gsettings, I derived this class form QGSettings.
 * It just represent the specific gsettings "org.ukui.style", and
 * there is no api difference from UKUIStyleSettings to QGSettings.
 */
class LIBQT5UKUISTYLESHARED_EXPORT UKUIStyleSettings : public QGSettings
{
    Q_OBJECT
public:
    UKUIStyleSettings();

    static UKUIStyleSettings *globalInstance();
};

#endif // UKUISTYLESETTINGS_H
