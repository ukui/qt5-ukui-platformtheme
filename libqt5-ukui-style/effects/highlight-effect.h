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

#ifndef HIGHLIGHTEFFECT_H
#define HIGHLIGHTEFFECT_H

#include <QObject>
#include <QStyleOption>

class QAbstractButton;

class HighLightEffect : public QObject
{
    Q_OBJECT
public:
    static bool setButtonIconHighlightEffect(QAbstractButton *button, bool set = true);
    static bool isWidgetIconUseHighlightEffect(const QWidget *w);

    static QPixmap generatePixmap(const QPixmap &pixmap,
                                  const QStyleOption *option,
                                  const QWidget *widget = nullptr,
                                  bool force = false);

private:
    explicit HighLightEffect(QObject *parent = nullptr);
};

#endif // HIGHLIGHTEFFECT_H
