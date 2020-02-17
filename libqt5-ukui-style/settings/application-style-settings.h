#ifndef APPLICATIONSTYLESETTINGS_H
#define APPLICATIONSTYLESETTINGS_H

#include <QSettings>

class QStyle;

/*!
 * \brief The ApplicationStyleSettings class
 * \details
 * This class is used to decide the style switch stretagies for independent application.
 * For example, you can choose the color scheme switch stretagy of an application, hold
 * the color in light or dark, or follow the system's palette.
 */
class ApplicationStyleSettings : public QSettings
{
    Q_OBJECT
public:
    enum ColorStretagy {
        System,
        Bright,
        Dark,
        Other
    };
    Q_ENUM(ColorStretagy)

    enum StyleStretagy {
        Default,
        Custom
    };
    Q_ENUM(StyleStretagy)

    static ApplicationStyleSettings *getInstance();

    ColorStretagy currentColorStretagy() {return m_color_stretagy;}
    StyleStretagy currentStyleStretagy() {return m_style_stretagy;}
    const QString currentCustomStyleName();

signals:
    void colorStretageChanged(const ColorStretagy &stretagy);
    void styleStretageChanged(const StyleStretagy &stretagy);

public slots:
    void setColorStretagy(ColorStretagy stretagy);
    void setStyleStretagy(StyleStretagy stretagy);
    void setCustomStyle(const QString &style);

private:
    explicit ApplicationStyleSettings(QObject *parent = nullptr);
    ~ApplicationStyleSettings() {}

    ColorStretagy m_color_stretagy;
    StyleStretagy m_style_stretagy;
    QString m_current_custom_style_name;
};

#endif // APPLICATIONSTYLESETTINGS_H
