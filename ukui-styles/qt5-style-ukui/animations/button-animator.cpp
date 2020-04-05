/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "button-animator.h"
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>

using namespace UKUI::Button;

ButtonAnimator::ButtonAnimator(QObject *parent) : QParallelAnimationGroup (parent)
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

bool ButtonAnimator::bindWidget(QWidget *w)
{
    if (qobject_cast<QToolButton*>(w)) {
        if (w->property("doNotAnimate").toBool())
            return false;
        m_widget = w;
        return true;
    }
    else if(qobject_cast<QPushButton*>(w))
    {
        if(w->property("doNotAnimate").toBool())
            return false;
        m_widget = w;
        return true;
    }
    else if(qobject_cast<QComboBox*>(w))
    {
        if(w->property("doNotAnimate").toBool())
            return false;
        m_widget = w;
        return true;
    }
    return false;
}

bool ButtonAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant ButtonAnimator::value(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentValue();
    else if(property == "SunKen")
        return m_sunken->currentValue();
    else
        return QVariant();
}

bool ButtonAnimator::isRunning(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->state() == Running;
    else if(property == "SunKen")
        return m_sunken->state() == Running;
    else
        return this->state() == Running;
}

bool ButtonAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setStartValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setStartValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ButtonAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setEndValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setEndValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ButtonAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if(property == "MouseOver")
    {
        m_mouseover->setDuration(duration);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setDuration(duration);
        return true;
    }
    else
    {
        return false;
    }
}

void ButtonAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "MouseOver")
        m_mouseover->setDirection(d);
    else if(property == "SunKen")
        m_sunken->setDirection(d);
    else
        return;
}
void ButtonAnimator::startAnimator(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->start();
    else if(property == "SunKen")
        m_sunken->start();
    else
        this->start();
}

void ButtonAnimator::stopAnimator(const QString &property)
{
    if(property == "MouseOver") {
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

int ButtonAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentTime();
    else if(property == "SunKen")
        return m_sunken->currentTime();
    else
        return this->currentTime();
}

int ButtonAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->duration();
    else if(property == "SunKen")
        return m_sunken->duration();
    else
        return this->duration();
}

QVariant ButtonAnimator::endValue(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->endValue();
    else if(property == "SunKen")
        return m_sunken->endValue();
    else
        return this->endValue();
}
