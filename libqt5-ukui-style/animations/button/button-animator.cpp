#include "button-animator.h"

#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QToolButton>



ButtonAnimator::ButtonAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

bool ButtonAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;
    if (qobject_cast<QToolButton*>(w) || qobject_cast<QPushButton*>(w) || qobject_cast<QComboBox*>(w)
            || qobject_cast<QSpinBox*>(w) || qobject_cast<QDoubleSpinBox*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_mouseover = new QVariantAnimation();
    m_mouseover->setStartValue(0.0);
    m_mouseover->setEndValue(1.0);
    m_mouseover->setDuration(100);
    m_mouseover->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_mouseover);

    m_sunken = new QVariantAnimation();
    m_sunken->setStartValue(0.0);
    m_sunken->setEndValue(1.0);
    m_sunken->setDuration(75);
    m_sunken->setEasingCurve(QEasingCurve::InCubic);
    addAnimation(m_sunken);

    connect(m_sunken, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_mouseover, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_sunken, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    connect(m_mouseover, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    return true;
}

bool ButtonAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    m_sunken->deleteLater();
    m_mouseover->deleteLater();

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;
}

QVariant ButtonAnimator::value(const QString &property)
{
    if (property == "MouseOver")
        return m_mouseover->currentValue();
    else if (property == "SunKen")
        return m_sunken->currentValue();
    else
        return QVariant();
}

bool ButtonAnimator::isRunning(const QString &property)
{
    if (property == "MouseOver")
        return m_mouseover->state() == Running;
    else if (property == "SunKen")
        return m_sunken->state() == Running;
    else
        return this->state() == Running;
}

bool ButtonAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "MouseOver") {
        m_mouseover->setStartValue(value);
        return true;
    } else if (property == "SunKen") {
        m_sunken->setStartValue(value);
        return true;
    }
    return false;
}

bool ButtonAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "MouseOver") {
        m_mouseover->setEndValue(value);
        return true;
    } else if (property == "SunKen") {
        m_sunken->setEndValue(value);
        return true;
    }
    return false;
}

bool ButtonAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "MouseOver") {
        m_mouseover->setDuration(duration);
        return true;
    } else if (property == "SunKen") {
        m_sunken->setDuration(duration);
        return true;
    }
    return false;
}

void ButtonAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "MouseOver")
        m_mouseover->setDirection(d);
    else if (property == "SunKen")
        m_sunken->setDirection(d);
    else
        return;
}

void ButtonAnimator::startAnimator(const QString &property)
{
    if (property == "MouseOver")
        m_mouseover->start();
    else if (property == "SunKen")
        m_sunken->start();
    else
        this->start();
}

void ButtonAnimator::stopAnimator(const QString &property)
{
    if (property == "MouseOver") {
        m_mouseover->stop();
        m_mouseover->setCurrentTime(0);
    } else if (property == "SunKen") {
        m_sunken->stop();
        m_sunken->setCurrentTime(0);
    } else {
        this->stop();
        this->setCurrentTime(0);
    }
}

int ButtonAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "MouseOver")
        return m_mouseover->currentTime();
    else if (property == "SunKen")
        return m_sunken->currentTime();
    else
        return this->currentTime();
}

int ButtonAnimator::totalAnimationDuration(const QString &property)
{
    if (property == "MouseOver")
        return m_mouseover->duration();
    else if(property == "SunKen")
        return m_sunken->duration();
    else
        return this->duration();
}
