#include "ukui-color-helper.h"

QColor UKUIColorHelper::get_SunKen_HighLight_Color()
{
    if (auto settings = UKUIStyleSettings::globalInstance()) {
        QString highlightMode = settings->get("highlightColor").toString();
        if (highlightMode == "daybreakBlue") {
            return QColor(41, 108, 217);
        } else if (highlightMode == "jamPurple") {
            return QColor(83, 29, 171);
        } else if (highlightMode == "magenta") {
            return QColor(196, 29, 127);
        } else if (highlightMode == "sunRed") {
            return QColor(207, 20, 34);
        } else if (highlightMode == "sunsetOrange") {
            return QColor(212, 107, 31);
        } else if (highlightMode == "dustGold") {
            return QColor(247, 173, 44);
        } else if (highlightMode == "polarGreen") {
            return QColor(56, 158, 33);
        } else if (highlightMode == "geekGrey") {
            return QColor(89, 89, 89);
        } else {
            return QColor(41, 108, 217);
        }
    } else {
        return QColor(41, 108, 217);
    }
}
