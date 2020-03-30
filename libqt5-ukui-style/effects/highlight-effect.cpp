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

#include "highlight-effect.h"

#include <QAbstractButton>
#include <QStyleOption>

#include <QPixmap>
#include <QPainter>

bool HighLightEffect::setButtonIconHighlightEffect(QAbstractButton *button, bool set)
{
    if (!button)
        return false;
    button->setProperty("useIconHighlightEffect", set);
    return true;
}

bool HighLightEffect::isWidgetIconUseHighlightEffect(const QWidget *w)
{
    if (w) {
        return w->property("useIconHighlightEffect").toBool();
    }
    return false;
}

QPixmap HighLightEffect::generatePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, bool force)
{
    if (force) {
        QPixmap target = pixmap;
        QPainter p(&target);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), option->palette.highlightedText());
        p.end();
        return target;
    }
    if (widget) {
        if (isWidgetIconUseHighlightEffect(widget)) {
            bool isEnable = option->state.testFlag(QStyle::State_Enabled);
            bool overOrDown = option->state.testFlag(QStyle::State_MouseOver)|option->state.testFlag(QStyle::State_Sunken);
            if (isEnable && overOrDown) {
                QPixmap target = pixmap;
                QPainter p(&target);
                p.setRenderHint(QPainter::Antialiasing);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                p.fillRect(target.rect(), option->palette.highlightedText());
                p.end();
                return target;
            }
        }
    } else {
        QPixmap target = pixmap;
        QPainter p(&target);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), option->palette.highlightedText());
        p.end();
        return target;
    }

    return pixmap;
}

HighLightEffect::HighLightEffect(QObject *parent) : QObject(parent)
{

}
