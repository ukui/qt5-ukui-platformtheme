#include "box-animation-helper.h"

#include "animations/box-animator.h"
#include <QWidget>

BoxAnimationHelper::BoxAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}


bool BoxAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::Box::BoxAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
    {
        animator->deleteLater();
    }
    else
    {
        m_animators->insert(w, animator);
    }
    return result;
}

bool BoxAnimationHelper::unregisterWidget(QWidget *w)
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

AnimatorIface *BoxAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}

