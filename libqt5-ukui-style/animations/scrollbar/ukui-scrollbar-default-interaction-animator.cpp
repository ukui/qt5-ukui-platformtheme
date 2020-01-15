#include "ukui-scrollbar-default-interaction-animator.h"
#include <QScrollBar>

#include <QVariantAnimation>

using namespace UKUI::ScrollBar;

DefaultInteractionAnimator::DefaultInteractionAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{
    setObjectName("ukui_scrollbar_default_interaction_animator");
}

bool DefaultInteractionAnimator::bindWidget(QWidget *w)
{
    if (qobject_cast<QScrollBar*>(w)) {
        m_widget = w;
        this->setParent(w);
        return true;
    }
    return false;
}

bool DefaultInteractionAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;
}

QVariant DefaultInteractionAnimator::value(const QString &property)
{
    if (property == "bg_opacity") {
        return m_bg_opacity->currentValue();
    } else if (property == "groove_width") {
        return m_groove_width->currentValue();
    } else if (property == "slider_opacity") {
        return m_slider_opacity->currentValue();
    } else {
        return QVariant();
    }
}

bool DefaultInteractionAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "bg_opacity") {
        m_bg_opacity->setStartValue(value);
        return true;
    } else if (property == "groove_width") {
        m_groove_width->setStartValue(value);
        return true;
    } else if (property == "slider_opacity") {
        m_slider_opacity->setStartValue(value);
        return true;
    } else {
        return false;
    }
}

bool DefaultInteractionAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "bg_opacity") {
        m_bg_opacity->setEndValue(value);
        return true;
    } else if (property == "groove_width") {
        m_groove_width->setEndValue(value);
        return true;
    } else if (property == "slider_opacity") {
        m_slider_opacity->setEndValue(value);
        return true;
    } else {
        return false;
    }
}

bool DefaultInteractionAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "bg_opacity") {
        m_bg_opacity->setDuration(duration);
        return true;
    } else if (property == "groove_width") {
        m_groove_width->setDuration(duration);
        return true;
    } else if (property == "slider_opacity") {
        m_groove_width->setDuration(duration);
        return true;
    } else {
        return false;
    }
}

void DefaultInteractionAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "bg_opacity") {
        m_bg_opacity->setDirection(d);
    } else if (property == "groove_width") {
        m_groove_width->setDirection(d);
    } else if (property == "slider_opacity") {
        m_slider_opacity->setDirection(d);
    } else {
        return;
    }
}

bool DefaultInteractionAnimator::isRunning(const QString &property)
{
    if (property == "bg_opacity") {
        return m_bg_opacity->state() == Running;
    } else if (property == "groove_width") {
        return m_groove_width->state() == Running;
    } else if (property == "slider_opacity") {
        return m_slider_opacity->state() == Running;
    } else {
        return this->state() == Running;
    }
}

void DefaultInteractionAnimator::startAnimator(const QString &property)
{
    if (property == "bg_opacity") {
        m_bg_opacity->start();
    } else if (property == "groove_width") {
        m_groove_width->start();
    } else if (property == "slider_opacity") {
        m_slider_opacity->start();
    } else {
        this->start();
    }
}

void DefaultInteractionAnimator::stopAnimator(const QString &property)
{
    if (property == "bg_opacity") {
        m_bg_opacity->stop();
    } else if (property == "groove_width") {
        m_groove_width->stop();
    } else if (property == "slider_opacity") {
        m_slider_opacity->stop();
    } else {
        this->stop();
    }
}

int DefaultInteractionAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "bg_opacity") {
        return m_bg_opacity->currentTime();
    } else if (property == "groove_width") {
        return m_groove_width->currentTime();
    } else if (property == "slider_opacity") {
        return m_slider_opacity->currentTime();
    } else {
        return this->currentTime();
    }
}
