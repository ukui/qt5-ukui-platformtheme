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

#include "tab-widget-animation-helper.h"
#include "tabwidget/ukui-tabwidget-default-slide-animator.h"

TabWidgetAnimationHelper::TabWidgetAnimationHelper(QObject *parent) : AnimationHelper(parent)
{

}

bool TabWidgetAnimationHelper::registerWidget(QWidget *w)
{
    auto animator = new UKUI::TabWidget::DefaultSlideAnimator;
    if (!animator->bindWidget(w))
        return false;
    m_animators->insert(w, animator);
    return true;
}

bool TabWidgetAnimationHelper::unregisterWidget(QWidget *w)
{
    auto animator = m_animators->value(w);
    if (animator) {
        animator->unboundWidget();
        delete animator;
    }
    m_animators->remove(w);
    return true;
}

AnimatorIface *TabWidgetAnimationHelper::animator(const QWidget *w)
{
    return m_animators->value(w);
}
