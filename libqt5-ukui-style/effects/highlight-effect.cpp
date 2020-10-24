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
#include <QtMath>

#include <QApplication>

#include <QDebug>

#define TORLERANCE 40

static QColor symbolic_color = Qt::gray;

void HighLightEffect::setSkipEffect(QWidget *w, bool skip)
{
    w->setProperty("skipHighlightIconEffect", skip);
}

bool HighLightEffect::isPixmapPureColor(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage();
    bool init = false;
    int red = 0;
    int green = 0;
    int blue = 0;
    qreal variance = 0;
    qreal mean = 0;
    qreal standardDeviation = 0;
    QVector<int> pixels;
    bool isPure = true;
    bool isFullyPure = true;
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() != 0) {
                int hue = color.hue();
                pixels<<hue;
                mean += hue;
                if (!init) {
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
                        if (isPure || isFullyPure) {
                            isPure = false;
                            isFullyPure = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (isPure)
        return true;

    mean = mean/pixels.count();
    for (auto hue : pixels) {
        variance += (hue - mean)*(hue - mean);
    }

    standardDeviation = qSqrt(variance/pixels.count());

    isFullyPure = standardDeviation == 0 || variance == 0;
    isPure = standardDeviation < 1 || variance == 0;

    return isPure;
}

bool HighLightEffect::setMenuIconHighlightEffect(QMenu *menu, HighLightMode hlmode, EffectMode mode)
{
    if (!menu)
        return false;

    menu->setProperty("useIconHighlightEffect", hlmode);
    menu->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::setViewItemIconHighlightEffect(QAbstractItemView *view, HighLightMode hlmode, EffectMode mode)
{
    if (!view)
        return false;

    view->viewport()->setProperty("useIconHighlightEffect", hlmode);
    view->viewport()->setProperty("iconHighlightEffectMode", mode);
    return true;
}

bool HighLightEffect::setButtonIconHighlightEffect(QAbstractButton *button, HighLightMode hlmode, EffectMode mode)
{
    if (!button)
        return false;

    button->setProperty("useIconHighlightEffect", hlmode);
    button->setProperty("iconHighlightEffectMode", mode);
    return true;
}

HighLightEffect::HighLightMode HighLightEffect::isWidgetIconUseHighlightEffect(const QWidget *w)
{
    if (w) {
        if (w->property("useIconHighlightEffect").isValid())
            return HighLightMode(w->property("useIconHighlightEffect").toInt());
    }
    return HighLightEffect::skipHighlight;
}

void HighLightEffect::setSymoblicColor(const QColor &color)
{
    qApp->setProperty("symbolicColor", color);
    symbolic_color = color;
}

void HighLightEffect::setWidgetIconFillSymbolicColor(QWidget *widget, bool fill)
{
    widget->setProperty("fillIconSymbolicColor", fill);
}

const QColor HighLightEffect::getCurrentSymbolicColor()
{
    QIcon symbolic = QIcon::fromTheme("window-new-symbolic");
    QPixmap pix = symbolic.pixmap(QSize(16, 16));
    QImage img = pix.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                symbolic_color = color;
                qDebug()<<"current symbol color:"<<color.name()<<QColor(Qt::gray).name();
                return color;
            }
        }
    }
    return symbolic_color;
}

const QColor HighLightEffect::defaultStyleDark()
{
    auto windowText = qApp->palette().windowText().color();
    qreal h, s, v;
    windowText.getHsvF(&h, &s, &v);
    return QColor::fromHsvF(h, s*0.85, v, 0.7);
}

QPixmap HighLightEffect::generatePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, bool force, EffectMode mode)
{
    if (pixmap.isNull())
        return pixmap;
    if (!(option->state & QStyle::State_Enabled))
        return pixmap;
    if (widget && !widget->isEnabled())
        return pixmap;

    QPixmap target = pixmap;
    bool isPurePixmap = isPixmapPureColor(pixmap);
    if (force) {
        if (!isPurePixmap)
            return pixmap;

        QPainter p(&target);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        if (option->state & QStyle::State_MouseOver ||
                option->state & QStyle::State_Selected ||
                option->state & QStyle::State_On ||
                option->state & QStyle::State_Sunken) {
            p.fillRect(target.rect(), option->palette.highlightedText());
        } else {
            if (mode == BothDefaultAndHighlit)
                p.fillRect(target.rect(), defaultStyleDark());
        }
        return target;
    }



    if (!widget)
        return pixmap;

    if (widget->property("skipHighlightIconEffect").isValid()) {
        bool skipEffect = widget->property("skipHighlightIconEffect").toBool();
        if (skipEffect)
            return pixmap;
    }

    if (widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toInt());
    }

    HighLightMode hlmode = isWidgetIconUseHighlightEffect(widget);

    if (hlmode == skipHighlight) {
        return pixmap;
    } else if (hlmode == HighlightEffect) {
        bool fillIconSymbolicColor = false;
        if (widget->property("fillIconSymbolicColor").isValid()) {
            fillIconSymbolicColor = widget->property("fillIconSymbolicColor").toBool();
        }

        bool isEnable = option->state.testFlag(QStyle::State_Enabled);
        bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
                option->state.testFlag(QStyle::State_Sunken) ||
                option->state.testFlag(QStyle::State_On) ||
                option->state.testFlag(QStyle::State_Selected);
        if (auto button = qobject_cast<const QAbstractButton *>(widget)) {
            if (button->isDown() || button->isChecked()) {
                overOrDown = true;
            }
        }

        if (qobject_cast<const QAbstractItemView *>(widget)) {
            if (!option->state.testFlag(QStyle::State_Selected))
                overOrDown = false;
        }

        if (isEnable && overOrDown) {
            if (fillIconSymbolicColor) {
                target = filledSymbolicColoredPixmap(pixmap, option->palette.highlightedText().color());
            }

            if (!isPurePixmap)
                return target;

            QPainter p(&target);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), option->palette.highlightedText());
            return target;
        } else {
            if (mode == BothDefaultAndHighlit) {
                QPixmap target = pixmap;

                if (fillIconSymbolicColor) {
                    target = filledSymbolicColoredPixmap(pixmap, option->palette.highlightedText().color());
                }

                if (!isPurePixmap)
                    return target;

                QPainter p(&target);
                p.setRenderHint(QPainter::Antialiasing);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.setCompositionMode(QPainter::CompositionMode_SourceIn);
                p.fillRect(target.rect(), defaultStyleDark());
                //p.end();
                return target;
            }
        }
    } else if (hlmode == ordinaryHighLight) {
        return ordinaryGeneratePixmap(pixmap, option, widget, mode);
    } else if (hlmode == hoverHighLight) {
        return hoverGeneratePixmap(pixmap, option, widget);
    } else if (hlmode == defaultHighLight) {
        return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);
    } else if (hlmode == filledSymbolicColorHighLight) {
        if (isPurePixmap) {
            return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);
        } else {
            return filledSymbolicColoredGeneratePixmap(pixmap, option, widget, mode);
        }
    } else {
        return pixmap;
    }

    return pixmap;
}

QPixmap HighLightEffect:: ordinaryGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (!isPixmapPureColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor color;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        color = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }

    if (widget && widget->property("iconHighlightEffectMode").isValid()) {
        mode = EffectMode(widget->property("iconHighlightEffectMode").toInt());
    }

    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(target.rect(), color.isValid() ? color : (mode ? defaultStyleDark() : option->palette.text()));
    return target;
}

QPixmap HighLightEffect::hoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget)
{
    if (!isPixmapPureColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor color;
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        color = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    bool isEnable = option->state.testFlag(QStyle::State_Enabled);
    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);
    if (auto button = qobject_cast<const QAbstractButton *>(widget)) {
        if (button->isDown() || button->isChecked())
            overOrDown = true;
    }
    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    }

    QPainter p(&target);
    if (isEnable && overOrDown) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), color.isValid() ? color : option->palette.highlightedText());
    }
    return target;
}

QPixmap HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (!isPixmapPureColor(pixmap))
        return pixmap;

    QPixmap target = pixmap;
    QColor defaultColor, hoverColor;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        defaultColor = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        hoverColor = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    bool isEnable = option->state.testFlag(QStyle::State_Enabled);
    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);
    if (auto button = qobject_cast<const QAbstractButton *>(widget)) {
        if (button->isDown() || button->isChecked())
            overOrDown = true;
    }
    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    }

    QPainter p(&target);
    if (isEnable && overOrDown) {
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), hoverColor.isValid() ? hoverColor : option->palette.highlightedText());
    } else {
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), defaultColor.isValid() ? defaultColor : (mode ? defaultStyleDark() : option->palette.text()));
    }
    return target;
}

QPixmap HighLightEffect::filledSymbolicColoredGeneratePixmap(const QPixmap &pixmap, const QStyleOption *option, const QWidget *widget, EffectMode mode)
{
    if (isPixmapPureColor(pixmap))
        return bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget, mode);

    QPixmap target = pixmap;
    QColor defaultColor, hoverColor;
    if (widget && widget->property("setIconHighlightEffectDefaultColor").isValid() && widget->property("setIconHighlightEffectDefaultColor").canConvert<QColor>()) {
        defaultColor = widget->property("setIconHighlightEffectDefaultColor").value<QColor>();
    }
    if (widget && widget->property("setIconHighlightEffectHoverColor").isValid() && widget->property("setIconHighlightEffectHoverColor").canConvert<QColor>()) {
        hoverColor = widget->property("setIconHighlightEffectHoverColor").value<QColor>();
    }

    bool isEnable = option->state.testFlag(QStyle::State_Enabled);
    bool overOrDown =  option->state.testFlag(QStyle::State_MouseOver) ||
            option->state.testFlag(QStyle::State_Sunken) ||
            option->state.testFlag(QStyle::State_On) ||
            option->state.testFlag(QStyle::State_Selected);
    if (auto button = qobject_cast<const QAbstractButton *>(widget)) {
        if (button->isDown() || button->isChecked())
            overOrDown = true;
    }
    if (qobject_cast<const QAbstractItemView *>(widget)) {
        if (!option->state.testFlag(QStyle::State_Selected))
            overOrDown = false;
    }

    if (isEnable && overOrDown) {
        return filledSymbolicColoredPixmap(target, hoverColor.isValid() ? hoverColor : option->palette.color(QPalette::HighlightedText));
    } else {
        return filledSymbolicColoredPixmap(target, defaultColor.isValid() ? defaultColor : (mode ? defaultStyleDark() : option->palette.color(QPalette::Text)));
    }

    return target;
}

HighLightEffect::HighLightEffect(QObject *parent) : QObject(parent)
{

}

QPixmap HighLightEffect::filledSymbolicColoredPixmap(const QPixmap &source, const QColor &baseColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                    qDebug()<<"fill"<<color<<baseColor;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}
