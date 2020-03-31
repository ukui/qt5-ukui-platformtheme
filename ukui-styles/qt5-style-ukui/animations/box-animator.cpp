#include "box-animator.h"

#include <QComboBox>

using namespace UKUI::Box;

BoxAnimator::BoxAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{
    m_mouseover = new QVariantAnimation(this);
    m_mouseover->setStartValue(0.0);
    m_mouseover->setEndValue(1.0);
    m_mouseover->setDuration(100);
    m_mouseover->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_mouseover);

    m_sunken = new QVariantAnimation(this);
    m_sunken->setStartValue(0.0);
    m_sunken->setEndValue(1.0);
    m_sunken->setDuration(75);
    m_sunken->setEasingCurve(QEasingCurve::InCubic);
    addAnimation(m_sunken);
}

bool BoxAnimator::bindWidget(QWidget *w)
{
    if (qobject_cast<QComboBox*>(w)) {
        if (w->property("doNotAnimate").toBool())
            return false;
        m_widget = w;
        return true;
    }
    return false;
}

bool BoxAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant BoxAnimator::value(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentValue();
    else if(property == "SunKen")
        return m_sunken->currentValue();
    else
        return QVariant();
}

bool BoxAnimator::isRunning(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->state() == Running;
    else if(property == "SunKen")
        return m_sunken->state() == Running;
    else
        return this->state() == Running;
}

bool BoxAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setStartValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setStartValue(value);
    }
    else
    {
        return false;
    }
}

bool BoxAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setEndValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setEndValue(value);
    }
    else
    {
        return false;
    }
}

bool BoxAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if(property == "MouseOver")
    {
        m_mouseover->setDuration(duration);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setDuration(duration);
    }
    else
    {
        return false;
    }

}

void BoxAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "MouseOver")
        m_mouseover->setDirection(d);
    else if(property == "SunKen")
        m_sunken->setDirection(d);
    else
        return;
}
void BoxAnimator::startAnimator(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->start();
    else if(property == "SunKen")
        m_sunken->start();
    else
        this->start();
}

void BoxAnimator::stopAnimator(const QString &property)
{
    if(property == "MouseOver")
    {
        m_mouseover->stop();
        m_mouseover->setCurrentTime(0);
    }
    else if(property == "SunKen")
    {
        m_sunken->stop();
        m_sunken->setCurrentTime(0);
    }
    else
    {
        this->stop();
        this->setCurrentTime(0);
    }
}

int BoxAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->currentTime();
    else if(property == "SunKen")
        m_sunken->currentTime();
    else
        this->currentTime();
}

int BoxAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->duration();
    else if(property == "SunKen")
        m_sunken->duration();
    else
        this->duration();
}

