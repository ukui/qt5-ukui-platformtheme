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

#include "box-animation-helper.h"

#include "animations/box-animator.h"
#include <QWidget>

BoxAnimationHelper::BoxAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}


bool BoxAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::Box::BoxAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
    {
        animator->deleteLater();
    }
    else
    {
        m_animators->insert(w, animator);
    }

    connect(w, &QWidget::destroyed, this, [=](){
       unregisterWidget(w);
    });

    return result;
}

bool BoxAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animator= m_animators->value(w);
    bool result = false;
    if (animator) {
        result = animator->unboundWidget();
        delete animator;
    }
    m_animators->remove(w);
    return result;
}

AnimatorIface *BoxAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}

