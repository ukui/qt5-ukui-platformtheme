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

#include "progressbar-animation-helper.h"

ProgressBarAnimationHelper::ProgressBarAnimationHelper(QObject *parent) : QObject(parent)
{
    animations = new QHash<QObject *, QVariantAnimation *>();
}



ProgressBarAnimationHelper::~ProgressBarAnimationHelper()
{
    delete animations;
}



void ProgressBarAnimationHelper::startAnimation(QVariantAnimation *animation)
{
    stopAnimation(animation->parent());
    connect(animation, SIGNAL(destroyed()), SLOT(_q_removeAnimation()), Qt::UniqueConnection);
    animations->insert(animation->parent(), animation);
    animation->start();
}



void ProgressBarAnimationHelper::stopAnimation(QObject *target)
{
    QVariantAnimation *animation = animations->take(target);
    if (animation) {
        animation->stop();
        delete animation;
    }
}



QVariantAnimation* ProgressBarAnimationHelper::animation(QObject *target)
{
    return animations->value(target);
}



void ProgressBarAnimationHelper::_q_removeAnimation()
{
    QObject *animation = this->sender();
    if (animation)
        animations->remove(animation->parent());
}
