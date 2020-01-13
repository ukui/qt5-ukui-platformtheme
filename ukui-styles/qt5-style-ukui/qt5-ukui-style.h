#ifndef QT5UKUISTYLE_H
#define QT5UKUISTYLE_H

#include <QProxyStyle>

class TabWidgetAnimationHelper;

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

private:
    TabWidgetAnimationHelper *m_tab_animation_helper;
};

#endif // QT5UKUISTYLE_H
