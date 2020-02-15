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

#include "scrollbar-animation-helper.h"

#include "animations/scrollbar/ukui-scrollbar-default-interaction-animator.h"

#include <QWidget>

ScrollBarAnimationHelper::ScrollBarAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}

bool ScrollBarAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::ScrollBar::DefaultInteractionAnimator;
    bool result = animator->bindWidget(w);
    if (!result)
        animator->deleteLater();
    return result;
}

bool ScrollBarAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animator = w->findChild<UKUI::ScrollBar::DefaultInteractionAnimator*>("ukui_scrollbar_default_interaction_animator", Qt::FindDirectChildrenOnly);
    bool result = false;
    if (animator) {
        result = animator->unboundWidget();
        animator->deleteLater();
    }
    return result;
}

AnimatorIface *ScrollBarAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}
