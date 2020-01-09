#include "ukui-style-settings.h"

static UKUIStyleSettings *global_instance = nullptr;

UKUIStyleSettings::UKUIStyleSettings() : QGSettings ("org.ukui.style", "/org/ukui/style/")
{
}

UKUIStyleSettings *UKUIStyleSettings::globalInstance()
{
    if (!global_instance){
        global_instance = new UKUIStyleSettings;
    }
    return global_instance;
}
