#ifndef UKUISTYLESETTINGS_H
#define UKUISTYLESETTINGS_H

#include "libqt5-ukui-style_global.h"
#include <QGSettings>

class LIBQT5UKUISTYLESHARED_EXPORT UKUIStyleSettings : public QGSettings
{
    Q_OBJECT
public:
    UKUIStyleSettings();

    static UKUIStyleSettings *globalInstance();
};

#endif // UKUISTYLESETTINGS_H
