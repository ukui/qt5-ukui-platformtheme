#ifndef SCROLLBARANIMATIONHELPER_H
#define SCROLLBARANIMATIONHELPER_H

#include <QObject>
#include "animation-helper.h"

class ScrollBarAnimationHelper : public AnimationHelper
{
    Q_OBJECT
public:
    explicit ScrollBarAnimationHelper(QObject *parent = nullptr);
    bool registerWidget(QWidget *w);
    bool unregisterWidget(QWidget *w);

signals:

public slots:
};

#endif // SCROLLBARANIMATIONHELPER_H
