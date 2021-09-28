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

#include "progressbar-animation.h"

ProgressBarAnimation::ProgressBarAnimation(QObject *parent) : QVariantAnimation (parent)
{
    init();
}



QWidget *ProgressBarAnimation::target()
{
    return qobject_cast<QWidget*>(parent());
}



void ProgressBarAnimation::init()
{
    this->setStartValue(0.0);
    this->setEndValue(1.0);
    this->setDuration(2500);
    this->setDirection(QAbstractAnimation::Forward);
    this->setEasingCurve(QEasingCurve::Linear);

    connect(this, &QVariantAnimation::valueChanged, target(), [=](QVariant value) {
        target()->repaint();
    });
    connect(this, &QVariantAnimation::finished, target(), [=]() {
       target()->repaint();
    });
}
