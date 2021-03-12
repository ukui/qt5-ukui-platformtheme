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

#include "ukui-scrollbar-default-interaction-animator.h"
#include <QScrollBar>

#include <QVariantAnimation>

#include <QDebug>

using namespace UKUI::ScrollBar;

DefaultInteractionAnimator::DefaultInteractionAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

/*!
 * \brief DefaultInteractionAnimator::bindWidget
 * \param w
 * \return
 *
 * \details
 * QObject has a feature that parent object can use findChild() method
 * getting a specific named child.
 *
 * I use QObject::setObjectName() set my animator and bind to a scroll bar.
 * So that i could not cache a hash or map to manage animators.
 *
 * \bug
 * Cause I use named QObject child to cache the animator for a scrollbar,
 * However there are some troubles for my unexcepted.
 *
 * For example, qt5 assistant's main view can not find child correctly.
 * I don't know if animator bind with child was been removed at some times.
 */
bool DefaultInteractionAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QScrollBar*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_groove_width = new QVariantAnimation(this);
    m_groove_width->setStartValue(0.0);
    m_groove_width->setEndValue(1.0);
    m_groove_width->setDuration(150);
    addAnimation(m_groove_width);

    m_slider_opacity = new QVariantAnimation(this);
    m_slider_opacity->setStartValue(0.0);
    m_slider_opacity->setEndValue(0.10);
    m_slider_opacity->setDuration(150);
    addAnimation(m_slider_opacity);

    m_sunken_silder_additional_opacity = new QVariantAnimation(this);
    m_sunken_silder_additional_opacity->setStartValue(0.0);
    m_sunken_silder_additional_opacity->setEndValue(0.10);
    m_sunken_silder_additional_opacity->setDuration(150);
    addAnimation(m_sunken_silder_additional_opacity);

    setObjectName("ukui_scrollbar_default_interaction_animator");

    connect(m_groove_width, &QVariantAnimation::valueChanged, w, [=]() {
       w->repaint();
    });
    connect(m_slider_opacity, &QVariantAnimation::valueChanged, w, [=]() {
       w->repaint();
    });
    connect(m_sunken_silder_additional_opacity, &QVariantAnimation::valueChanged, w, [=]() {
       w->repaint();
    });
    connect(m_groove_width, &QVariantAnimation::finished, w, [=]() {
       w->repaint();
    });
    connect(m_slider_opacity, &QVariantAnimation::finished, w, [=]() {
       w->repaint();
    });
    connect(m_sunken_silder_additional_opacity, &QVariantAnimation::finished, w, [=]() {
       w->repaint();
    });

    return true;
}

bool DefaultInteractionAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    m_groove_width->deleteLater();
    m_slider_opacity->deleteLater();
    m_sunken_silder_additional_opacity->deleteLater();

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;
}

QVariant DefaultInteractionAnimator::value(const QString &property)
{
    if (property == "groove_width") {
        return m_groove_width->currentValue();
    } else if (property == "slider_opacity") {
        return m_slider_opacity->currentValue();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->currentValue();
    } else {
        return QVariant();
    }
}

bool DefaultInteractionAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "groove_width") {
        m_groove_width->setStartValue(value);
        return true;
    } else if (property == "slider_opacity") {
        m_slider_opacity->setStartValue(value);
        return true;
    } else if (property == "additional_opacity")
    {
        m_sunken_silder_additional_opacity->setStartValue(value);
        return true;
    } else {
        return false;
    }
}

bool DefaultInteractionAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "groove_width") {
        m_groove_width->setEndValue(value);
        return true;
    } else if (property == "slider_opacity") {
        m_slider_opacity->setEndValue(value);
        return true;
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setEndValue(value);
        return true;
    } else {
        return false;
    }
}

bool DefaultInteractionAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "groove_width") {
        m_groove_width->setDuration(duration);
        return true;
    } else if (property == "slider_opacity") {
        m_groove_width->setDuration(duration);
        return true;
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setDuration(duration);
        return true;
    } else {
        return false;
    }
}

void DefaultInteractionAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "groove_width") {
        m_groove_width->setDirection(d);
    } else if (property == "slider_opacity") {
        m_slider_opacity->setDirection(d);
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setDirection(d);
    } else {
        return;
    }
}

bool DefaultInteractionAnimator::isRunning(const QString &property)
{
    if (property == "groove_width") {
        return m_groove_width->state() == Running;
    } else if (property == "slider_opacity") {
        return m_slider_opacity->state() == Running;
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->state() == Running;
    } else {
        return this->state() == Running;
    }
}

void DefaultInteractionAnimator::startAnimator(const QString &property)
{
    if (property == "groove_width") {
        m_groove_width->start();
    } else if (property == "slider_opacity") {
        m_slider_opacity->start();
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->start();
    } else {
        this->start();
    }
}

void DefaultInteractionAnimator::stopAnimator(const QString &property)
{
    if (property == "groove_width") {
        m_groove_width->stop();
    } else if (property == "slider_opacity") {
        m_slider_opacity->stop();
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->stop();
    } else {
        this->stop();
    }
}

int DefaultInteractionAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "groove_width") {
        return m_groove_width->currentTime();
    } else if (property == "slider_opacity") {
        return m_slider_opacity->currentTime();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->currentTime();
    } else {
        return this->currentTime();
    }
}

int DefaultInteractionAnimator::totalAnimationDuration(const QString &property)
{
    if (property == "groove_width") {
        return m_groove_width->duration();
    } else if (property == "slider_opacity") {
        return m_slider_opacity->duration();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->duration();
    } else {
        return this->duration();
    }
}
