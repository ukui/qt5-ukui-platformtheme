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

#include "blur-helper.h"
#include "ukui-style-settings.h"
#include <QWidget>
#include <KWindowEffects>
#include <QGSettings>
#include <QVariant>
#include <QEvent>
#include <QIcon>
#include <QPainterPath>

#include <QMenu>

#include "black-list.h"

#include <QApplication>
#include <QX11Info>

#include <QDebug>

BlurHelper::BlurHelper(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "enabledGlobalBlur") {
                bool enable = settings->get(key).toBool();
                this->onBlurEnableChanged(enable);
            }
        });
        bool enable = settings->get("enabledGlobalBlur").toBool();
        this->onBlurEnableChanged(enable);

        if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind))
            confirmBlurEnableDelay();
    }
    m_timer.setSingleShot(true);
    m_timer.setInterval(100);
}

bool BlurHelper::eventFilter(QObject *obj, QEvent *e)
{
    if (!m_blur_enable)
        return false;

    QWidget* widget = qobject_cast<QWidget*>(obj);

    if (!widget || !widget->isWindow())
        return false;

    //FIXME:
    //qDebug()<<e->type()<<obj;
    //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
    switch (e->type()) {
    case QEvent::UpdateRequest: {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        delayUpdate(widget, true);
        break;
    }
    case QEvent::LayoutRequest:
    {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        delayUpdate(widget);
        break;
    }
    case QEvent::Hide: {
        //QWidget* widget = qobject_cast<QWidget*>(obj);
        KWindowEffects::enableBlurBehind(widget->winId(), false);
    }

    default:
        break;
    }
    return false;
}

/*!
 * \brief BlurHelper::registerWidget
 * \param widget
 * \note
 *
 * we can't blur a widget before it shown, because some times
 * there might be problems about window painting.
 * this usually happend on a widget which has graphics effect.
 *
 * to avoid them, never try get winid and do a blur for that case.
 */
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
void BlurHelper::registerWidget(QWidget *widget)
{
    // FIXME: how to blur window on wayland?
    if (!QX11Info::isPlatformX11())
        return;

    if (!widget)
        return;

    if (shouldSkip(widget))
        return;
    if (isApplicationInBlackList())
        return;
    if (widget->property("doNotBlur").toBool())
        return;
    if (!m_blur_widgets.contains(widget)) {
        m_blur_widgets<<widget;
        //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
        /*!
          \note
          never try enableBlurBehind when register widget.
          it might cause serious problems about paint.
          usually it happend with a widget which has graphics effect.
          */
//        if (!widget->mask().isEmpty()) {
//            KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
//        } else {
//            KWindowEffects::enableBlurBehind(widget->winId(), true);
//        }

        connect(widget, &QWidget::destroyed, this, [=]() {
            this->onWidgetDestroyed(widget);
        });
    }
    widget->removeEventFilter(this);
    widget->installEventFilter(this);

    if (!widget->mask().isEmpty()) {
        widget->update(widget->mask());
    } else {
        widget->update();
    }
}

void BlurHelper::unregisterWidget(QWidget *widget)
{
    if (!QX11Info::isPlatformX11())
        return;

    if (!widget)
        return;

    if (shouldSkip(widget))
        return;
    if (isApplicationInBlackList())
        return;
    if (widget->property("doNotBlur").toBool())
        return;
    m_blur_widgets.removeOne(widget);
    widget->removeEventFilter(this);
    if (widget->winId() > 0)
        KWindowEffects::enableBlurBehind(widget->winId(), false);
}
#endif

bool BlurHelper::isApplicationInBlackList()
{
    return blackList().contains(qAppName());
}

const QStringList BlurHelper::blackList()
{
    return blackAppListWithBlurHelper();
}

bool BlurHelper::shouldSkip(QWidget *w)
{
    if (w->property("useSystemStyleBlur").isValid()) {
        return !w->property("useSystemStyleBlur").toBool();
    }

    bool skip = true;
    if (w->inherits("QComboBoxPrivateContainer"))
        return true;

    if (w->inherits("QMenu") || w->inherits("QTipLabel")) {
        return false;
    }

//    if (w->inherits("QTipLabel"))
//        return true;

    return skip;
}

void BlurHelper::onBlurEnableChanged(bool enable)
{
    m_blur_enable = enable;

    if (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) && enable) {
        qApp->setProperty("blurEnable", true);
    } else {
        qApp->setProperty("blurEnable", false);
    }
    for (auto widget : qApp->allWidgets()) {
        widget->update();
        if (m_blur_widgets.contains(widget)) {
            if (widget->winId() > 0)
                KWindowEffects::enableBlurBehind(widget->winId(), enable);
        }
    }
//    QTimer::singleShot(100, this, [=](){
//        for (auto widget : m_blur_widgets) {
//            if (!widget)
//                continue;
//            if (widget->winId() > 0)
//                KWindowEffects::enableBlurBehind(widget->winId(), enable);
//        }
//    });
}

void BlurHelper::onWidgetDestroyed(QWidget *widget)
{
    widget->removeEventFilter(this);
    m_blur_widgets.removeOne(widget);
    //unregisterWidget(widget);
}

void BlurHelper::delayUpdate(QWidget *w, bool updateBlurRegionOnly)
{
    if (w->winId() <= 0)
        return;

    m_update_list.append(w);
    if (!m_timer.isActive()) {
        for (auto widget : m_update_list) {
            // cast to widget and check
            //KWindowEffects::enableBlurBehind(widget->winId(), false);

            if (!widget)
                continue;

            if (widget->winId() <= 0)
                continue;

            bool hasMask = false;
            if (widget->mask().isNull())
                hasMask = true;

            QVariant regionValue = widget->property("blurRegion");
            QRegion region = qvariant_cast<QRegion>(regionValue);

            if (widget->inherits("QMenu")) {
                //skip menu which has style sheet.
                if (!widget->styleSheet().isEmpty() || qApp->styleSheet().contains("QMenu")) {
                    break;
                }
                QPainterPath path;
                path.addRoundedRect(widget->rect().adjusted(+5,+5,-5,-5), 6, 6);
                KWindowEffects::enableBlurBehind(widget->winId(), true, path.toFillPolygon().toPolygon());
                if (!updateBlurRegionOnly)
                    widget->update();
                break;
            }

            if (widget->inherits("QTipLabel")) {
                QPainterPath path;
                path.addRoundedRect(widget->rect().adjusted(+3,+3,-3,-3),4, 4);
                KWindowEffects::enableBlurBehind(widget->winId(), true, path.toFillPolygon().toPolygon());
                if (!updateBlurRegionOnly)
                    widget->update();
                break;
            }

            if (!hasMask && region.isEmpty())
                break;

            //qDebug()<<regionValue<<region;
            //qDebug()<<widget->metaObject()->className()<<widget->geometry()<<widget->mask();
            if (!region.isEmpty()) {
                //qDebug()<<"blur region"<<region;
                KWindowEffects::enableBlurBehind(widget->winId(), true, region);
                if (!updateBlurRegionOnly)
                    widget->update();
            } else {
                //qDebug()<<widget->mask();
                KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
                if (!updateBlurRegionOnly)
                    widget->update(widget->mask());
            }

            //NOTE: we can not setAttribute Qt::WA_TranslucentBackground here,
            //because the window is about to be shown.
            //widget->setAttribute(Qt::WA_TranslucentBackground);
            //KWindowEffects::enableBlurBehind(widget->winId(), true);
            //widget->update();
        }
        m_update_list.clear();
    } else {
        m_timer.start();
    }
}

void BlurHelper::confirmBlurEnableDelay()
{
    QTimer::singleShot(3000, this, [=]() {
        bool enable = m_blur_enable;
        if (KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) && enable) {
            qApp->setProperty("blurEnable", true);
        } else {
            qApp->setProperty("blurEnable", false);
        }
        for (auto widget : qApp->allWidgets()) {
            widget->update();
            if (m_blur_widgets.contains(widget)) {
                if (widget->winId() > 0)
                    KWindowEffects::enableBlurBehind(widget->winId(), enable);
            }
        }
//        QTimer::singleShot(100, this, [=](){
//            for (auto widget : m_blur_widgets) {
//                if (!widget)
//                    continue;
//                if (widget->winId() > 0)
//                    KWindowEffects::enableBlurBehind(widget->winId(), enable);
//            }
//        });
    });
}
