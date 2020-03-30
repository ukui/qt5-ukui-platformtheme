#ifndef BOXANIMATIONHELPER_H
#define BOXANIMATIONHELPER_H

#include <QObject>
#include "animation-helper.h"
#include "animator-iface.h"
#include "box-animator.h"

class BoxAnimationHelper : public AnimationHelper
{
    Q_OBJECT
public:
    BoxAnimationHelper(QObject *parent = nullptr);
    bool registerWidget(QWidget *w);
    bool unregisterWidget(QWidget *w);

    AnimatorIface *animator(const QWidget *w);

signals:

public slots:
};

#endif // BOXANIMATIONHELPER_H
