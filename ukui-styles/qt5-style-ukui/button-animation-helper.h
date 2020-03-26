#ifndef BUTTONANIMATIONHELPER_H
#define BUTTONANIMATIONHELPER_H

#include <QObject>
#include "animation-helper.h"
#include "animator-iface.h"
#include "button-animator.h"

class ButtonAnimationHelper: public AnimationHelper
{
    Q_OBJECT
public:
    ButtonAnimationHelper(QObject *parent = nullptr);
    bool registerWidget(QWidget *w);
    bool unregisterWidget(QWidget *w);

    AnimatorIface *animator(const QWidget *w);

signals:

public slots:
};

#endif // BUTTONANIMATIONHELPER_H
