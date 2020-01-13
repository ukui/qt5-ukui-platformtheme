#ifndef TABWIDGETANIMATIONHELPER_H
#define TABWIDGETANIMATIONHELPER_H

#include <QObject>
#include "animation-helper.h"

class TabWidgetAnimationHelper : public AnimationHelper
{
    Q_OBJECT
public:
    explicit TabWidgetAnimationHelper(QObject *parent = nullptr);

    bool registerWidget(QWidget *w);
    bool unregisterWidget(QWidget *w);
};

#endif // TABWIDGETANIMATIONHELPER_H
