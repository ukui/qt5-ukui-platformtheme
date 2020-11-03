#ifndef UKUISTYLE_H
#define UKUISTYLE_H

#include <ukui-style-settings.h>

#include <private/qfusionstyle_p.h>
#include <QApplication>
#include <QStyleOption>



class UKUIStyle : public QFusionStyle
{

public:
    enum StyleType {
        udefault,
        udark,
        ulight
    };

    UKUIStyle(StyleType type = udefault);

    QPalette standardPalette() const;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
private:
    StyleType m_type;

    const QStringList useLightPalette() const;
    const QStringList useDefaultPalette() const;
};

#endif // UKUISTYLE_H
