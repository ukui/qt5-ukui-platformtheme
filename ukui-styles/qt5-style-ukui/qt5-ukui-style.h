#ifndef QT5UKUISTYLE_H
#define QT5UKUISTYLE_H

#include <QProxyStyle>

class TabWidgetAnimationHelper;
class ScrollBarAnimationHelper;

class Qt5UKUIStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit Qt5UKUIStyle(bool dark = false);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

private:
    TabWidgetAnimationHelper *m_tab_animation_helper;
    ScrollBarAnimationHelper *m_scrollbar_animation_helper;

    bool m_use_dark_palette = false;
};

#endif // QT5UKUISTYLE_H
