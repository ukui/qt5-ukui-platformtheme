#include "ukui-tabwidget-default-slide-animator-factory.h"
#include "ukui-tabwidget-default-slide-animator.h"

using namespace UKUI::TabWidget;

DefaultSlideAnimatorFactory::DefaultSlideAnimatorFactory(QObject *parent) : QObject(parent)
{

}

UKUITabWidgetAnimatorIface *DefaultSlideAnimatorFactory::createAnimator()
{
    return new DefaultSlideAnimator;
}
