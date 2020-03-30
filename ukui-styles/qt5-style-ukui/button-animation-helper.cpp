#include "button-animation-helper.h"
#include "animations/button-animator.h"
#include <QWidget>

ButtonAnimationHelper::ButtonAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}


bool ButtonAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::Button::ButtonAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
        animator->deleteLater();

    m_animators->insert(w, animator);
    return result;
}

bool ButtonAnimationHelper::unregisterWidget(QWidget *w)
{
   auto animator= m_animators->value(w);
    bool result = false;
    if (animator) {
        result = animator->unboundWidget();
        delete animator;
    }
    m_animators->remove(w);
    return result;
}

AnimatorIface *ButtonAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}
