#ifndef BUTTONANIMATIONHELPER_H
#define BUTTONANIMATIONHELPER_H

#include "animation-helper.h"



class ButtonAnimationHelper: public AnimationHelper
{
public:
    ButtonAnimationHelper(QObject *parent = nullptr);
    bool registerWidget(QWidget *w);
    bool unregisterWidget(QWidget *w);

    AnimatorIface *animator(const QWidget *w);
};

#endif // BUTTONANIMATIONHELPER_H
