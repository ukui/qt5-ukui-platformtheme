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

#include <QAbstractItemView>
#include <QAbstractButton>
#include <QMenu>
#include <QStyleOption>

#include <QPixmap>
#include <QPainter>

#include <QImage>

#include <QDebug>

#define TORLERANCE 32

void HighLightEffect::setSkipEffect(QWidget *w, bool skip)
{
    w->setProperty("skipHighlightIconEffect", skip);
}

bool HighLightEffect::isPixmapPureColor(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage();
    QColor tmp = Qt::transparent;
    bool init = false;
    int red = 0;
    int green = 0;
    int blue = 0;
    bool isPure = true;
    bool isFullyPure = true;
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() != 0) {
                if (!init) {
                    tmp = color;
                    red = color.red();
                    green = color.green();
                    blue = color.blue();
                    init = true;
                } else {
                    color.setAlpha(255);
                    int r = color.red();
                    int g = color.green();
                    int b = color.blue();
                    int dr = qAbs(r - red);
                    int dg = qAbs(g - green);
                    int db = qAbs(b - blue);
                    bool same = dr < TORLERANCE && dg < TORLERANCE && db < TORLERANCE;
                    if (isFullyPure) {
                        if (dr > 0 || dg > 0 || db > 0) {
                            isFullyPure = false;
                        }
                    }
                    if (!same) {
                        return false;
                    }
                }
            }
        }
    }
    return isPure;
}

bool HighLightEffect::setMenuIconHighlightEffect(QMenu *menu, bool set, HighLightEffect::EffectMode mode)
{
    if (!menu)
        return false;

    menu->setProperty("useIconHighlightEffect", set);
    menu->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::setViewItemIconHighlightEffect(QAbstractItemView *view, bool set, HighLightEffect::EffectMode mode)
{
    if (!view)
        return false;

    view->viewport()->setProperty("useIconHighlightEffect", set);
    view->viewport()->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::setButtonIconHighlightEffect(QAbstractButton *button, bool set, EffectMode mode)
{
    if (!button)
        return false;

    button->setProperty("useIconHighlightEffect", set);
    button->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::isWidgetIconUseHighlightEffect(const QWidget *w)
{
    if (w) {
        return w->property("useIconHighlightEffect").toBool();
    }
    return false;
}

QPixmap HighLightEffect::generatePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, bool force, EffectMode mode)
{
    if (widget) {
        if (widget->property("skipHighlightIconEffect").isValid()) {
            bool skipEffect = widget->property("skipHighlightIconEffect").toBool();
            if (skipEffect)
                return pixmap;
        }
    }
    if (force) {
        if (!isPixmapPureColor(pixmap))
            return pixmap;

        QPixmap target = pixmap;
        QPainter p(&target);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        if (option->state & QStyle::State_MouseOver |
                option->state & QStyle::State_Selected |
                option->state & QStyle::State_On |
                option->state & QStyle::State_Sunken) {
            p.fillRect(target.rect(), option->palette.highlightedText());
        } else {
            if (mode == BothDefaultAndHighlit)
                p.fillRect(target.rect(), option->palette.shadow());
        }
        p.end();
        return target;
    }

    if (!isPixmapPureColor(pixmap))
        return pixmap;

    if (widget) {
        if (isWidgetIconUseHighlightEffect(widget)) {
            if (widget->property("iconHighlightEffectMode").isValid()) {
                mode = qvariant_cast<EffectMode>(widget->property("iconHighlightEffectMode"));
            }

            bool isEnable = option->state.testFlag(QStyle::State_Enabled);
            bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) |
                    option->state.testFlag(QStyle::State_Sunken) |
                    option->state.testFlag(QStyle::State_On) |
                    option->state.testFlag(QStyle::State_Selected);

            if (isEnable && overOrDown) {
                QPixmap target = pixmap;
                QPainter p(&target);
                p.setRenderHint(QPainter::Antialiasing);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                p.fillRect(target.rect(), option->palette.highlightedText());
                p.end();
                return target;
            } else {
                if (mode == BothDefaultAndHighlit) {
                    QPixmap target = pixmap;
                    QPainter p(&target);
                    p.setRenderHint(QPainter::Antialiasing);
                    p.setRenderHint(QPainter::SmoothPixmapTransform);
                    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                    p.fillRect(target.rect(), option->palette.shadow());
                    p.end();
                    return target;
                }
            }
        }
    } else {
        bool isEnable = option->state.testFlag(QStyle::State_Enabled);
        bool overOrDown = option->state.testFlag(QStyle::State_MouseOver) |
                option->state.testFlag(QStyle::State_Sunken) |
                option->state.testFlag(QStyle::State_Selected) |
                option->state.testFlag(QStyle::State_On);
        if (isEnable && overOrDown) {
            QPixmap target = pixmap;
            QPainter p(&target);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), option->palette.highlightedText());
            p.end();
            return target;
        } else {
            if (mode == BothDefaultAndHighlit) {
                QPixmap target = pixmap;
                QPainter p(&target);
                p.setRenderHint(QPainter::Antialiasing);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                p.fillRect(target.rect(), option->palette.shadow());
                p.end();
                return target;
            }
        }
    }

    return pixmap;
}

HighLightEffect::HighLightEffect(QObject *parent) : QObject(parent)
{

}
