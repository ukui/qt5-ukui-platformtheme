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
    auto animator = w->findChild<UKUI::Button::ButtonAnimator*>("ukui_scrollbar_default_interaction_animator", Qt::FindDirectChildrenOnly);
    bool result = false;
    if (animator) {
        result = animator->unboundWidget();
        animator->deleteLater();
    }
    m_animators->remove(w);
    return result;
}

AnimatorIface *ButtonAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}
