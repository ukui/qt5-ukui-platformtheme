#include "proxy-style-plugin.h"
#include "proxy-style.h"
#include "ukui-style-settings.h"

#include <QApplication>
#include <QStyleFactory>

#include <QDebug>

using namespace UKUI;

ProxyStylePlugin::ProxyStylePlugin()
{
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        connect(settings, &UKUIStyleSettings::changed, this, [=](const QString &key){
            if (key == "styleName") {
                auto styleName = settings->get("styleName").toString();
                if (m_current_style_name == styleName)
                    return;

                if (styleName == "ukui") {
                    styleName = "ukui-white";
                }
                if (!QStyleFactory::keys().contains(styleName)) {
                    styleName = "fusion";
                }
                QApplication::setStyle(new ProxyStyle(styleName));
            }
        });
    }
}

QStyle *ProxyStylePlugin::create(const QString &key)
{
    if (key == "ukui") {
        //FIXME:
        //get current style, fusion for invalid.
        if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
            m_current_style_name = UKUIStyleSettings::globalInstance()->get("styleName").toString();
            return new ProxyStyle(UKUIStyleSettings::globalInstance()->get("styleName").toString());
        }
        qDebug()<<"ukui create proxy style";
        return new ProxyStyle(nullptr);
    }
    qDebug()<<"ukui create proxy style: null";
    return nullptr;
}
