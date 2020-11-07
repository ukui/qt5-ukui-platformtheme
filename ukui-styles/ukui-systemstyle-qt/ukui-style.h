#ifndef UKUISTYLE_H
#define UKUISTYLE_H

#include <ukui-style-settings.h>
#include <button-animation-helper.h>

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

    QPalette standardPalette() const override;

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;

    void polish(QPalette &palette) override;
    void polish(QApplication *applicaton) override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    void unpolish(QApplication *application) override;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr,  const QWidget *widget = nullptr) const override;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &size, const QWidget *widget = nullptr) const override;
private:
    StyleType m_type;

    ButtonAnimationHelper *m_button_animation_helper;

    const QStringList useLightPalette() const;
    const QStringList useDefaultPalette() const;
    void setHighLightPalette(QPalette &palette, QString mode) const;
};

#endif // UKUISTYLE_H
