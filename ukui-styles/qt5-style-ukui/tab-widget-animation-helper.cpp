#include "tab-widget-animation-helper.h"
#include "tabwidget/ukui-tabwidget-default-slide-animator.h"

TabWidgetAnimationHelper::TabWidgetAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}

bool TabWidgetAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::TabWidget::DefaultSlideAnimator;
    if (!animator->bindWidget(w))
        return false;
    m_animators->insert(w, animator);
    return true;
}

bool TabWidgetAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animator = m_animators->value(w);
    if (animator) {
        animator->unboundWidget();
    }
    m_animators->remove(w);
    return true;
}
