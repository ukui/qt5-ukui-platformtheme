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

#ifndef UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H
#define UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H

#include <QParallelAnimationGroup>
#include "animator-iface.h"

class QVariantAnimation;

namespace UKUI {

namespace ScrollBar {

class DefaultInteractionAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
public:
    explicit DefaultInteractionAnimator(QObject *parent = nullptr);

    bool bindWidget(QWidget *w);
    bool unboundWidget();
    QWidget *boundedWidget() {return m_widget;}

    QVariant value(const QString &property);
    bool isRunning(const QString &property = nullptr);
    bool setAnimatorStartValue(const QString &property, const QVariant &value);
    bool setAnimatorEndValue(const QString &property, const QVariant &value);

    bool setAnimatorDuration(const QString &property, int duration);
    void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true);
    void startAnimator(const QString &property = nullptr);
    void stopAnimator(const QString &property = nullptr);
    int currentAnimatorTime(const QString &property = nullptr);
    int totalAnimationDuration(const QString &property);

private:
    QWidget *m_widget = nullptr;

    QVariantAnimation *m_bg_opacity;
    QVariantAnimation *m_groove_width;
    QVariantAnimation *m_slider_opacity;
    QVariantAnimation *m_sunken_silder_additional_opacity;
};

}

}

#endif // UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H
