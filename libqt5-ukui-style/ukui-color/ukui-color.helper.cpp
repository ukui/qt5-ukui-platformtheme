#include "ukui-color-helper.h"

QColor UKUIColorHelper::get_SunKen_HighLight_Color()
{
    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString highlightMode = settings->get("highlightColor").toString();
        if (highlightMode == "daybreak-blue") {
            return QColor(41, 108, 217);
        } else if (highlightMode == "jam-purple") {
            return QColor(83, 29, 171);
        } else if (highlightMode == "magenta") {
            return QColor(196, 29, 127);
        } else if (highlightMode == "sun-red") {
            return QColor(207, 20, 34);
        } else if (highlightMode == "sunset-orange") {
            return QColor(212, 107, 31);
        } else if (highlightMode == "dust-gold") {
            return QColor(247, 173, 44);
        } else if (highlightMode == "polar-green") {
            return QColor(56, 158, 33);
        } else if (highlightMode == "geek-grey") {
            return QColor(89, 89, 89);
        } else {
            return QColor(41, 108, 217);
        }
    } else {
        return QColor(41, 108, 217);
    }
}



QColor UKUIColorHelper::get_SunKen_Button_Color()
{
    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString paletteMode = settings->get("styleName").toString();
        if (paletteMode == "ukui3.1-dark") {
            return QColor(46, 46, 46);
        }
    }
    return QColor(217, 217, 217);
}
