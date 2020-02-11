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

#ifndef UKUITABWIDGETANIMATORIFACE_H
#define UKUITABWIDGETANIMATORIFACE_H

#include <QTabWidget>
#include "animator-iface.h"

/*!
 * \brief The UKUITabWidgetAnimatorIface class
 * \details
 * This class define the interface for doing a QTabWidget's animation.
 * a tabwidget animator should bind only one tabwidget with bindTabWidget(),
 * and can be unbounded with unboundTabWidget().
 *
 * Animator is created by AnimatorPlugin, which is another interface's implement
 * of UKUI style animation's frameworks.
 *
 * \see UKUITabWidgetAnimatorPluginIface
 */
class UKUITabWidgetAnimatorIface : public AnimatorIface
{
public:
    virtual ~UKUITabWidgetAnimatorIface() {}

    virtual bool bindWidget(QWidget *w) {
        return bindTabWidget(qobject_cast<QTabWidget *>(w));
    }

    virtual bool unboundWidget() {
        return unboundTabWidget();
    }

    /*!
     * \brief bindTabWidget
     * \param w widget should be bound.
     * \return true if successed.
     * \details
     * this method is used for binding a animator instance for a tab widget.
     * You have to implement this function in your own implement class.
     */
    virtual bool bindTabWidget(QTabWidget *w) = 0;

    /*!
     * \brief unboundTabWidget
     * \return true if successed.
     * \details
     * this method is used to unbound the animator instance and tab widget.
     * You have to implement this function in your own implement class.
     */
    virtual bool unboundTabWidget() = 0;
};

#endif // UKUITABWIDGETANIMATORIFACE_H
