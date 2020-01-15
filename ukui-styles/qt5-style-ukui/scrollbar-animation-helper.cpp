#include "scrollbar-animation-helper.h"

#include "animations/scrollbar/ukui-scrollbar-default-interaction-animator.h"

#include <QWidget>

ScrollBarAnimationHelper::ScrollBarAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}

bool ScrollBarAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::ScrollBar::DefaultInteractionAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
        animator->deleteLater();
    return result;
}

bool ScrollBarAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animator = w->findChild<UKUI::ScrollBar::DefaultInteractionAnimator*>("ukui_scrollbar_default_interaction_animator", Qt::FindDirectChildrenOnly);
    bool result = false;
    if (animator) {
        result = animator->unboundWidget();
        animator->deleteLater();
    }
    return result;
}
