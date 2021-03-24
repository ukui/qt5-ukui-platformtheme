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

#include "proxy-style.h"
#include <QWidget>
#include "blur-helper.h"
#include "gesture-helper.h"
#include "window-manager.h"
#include "application-style-settings.h"

#include "ukui-style-settings.h"

#include <QApplication>
#include <QMenu>

#include <QWindow>

#include <QLabel>
#include <QWizardPage>

#include <QStyleHints>

#include <QDebug>

using namespace UKUI;

ProxyStyle::ProxyStyle(const QString &key) : QProxyStyle(key == nullptr? "fusion": key)
{
    auto settings = UKUIStyleSettings::globalInstance();
//    m_use_custom_highlight_color = settings->get("useCustomHighlightColor").toBool();
//    m_custom_highlight_color = QColor(settings->get("customHighlightColor").toString());
    m_blink_cursor = settings->get("cursorBlink").toBool();
    m_blink_cursor_time = settings->get("cursorBlinkTime").toInt();
    qApp->styleHints()->setCursorFlashTime(m_blink_cursor_time);
    connect(settings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "cursorBlink") {
            m_blink_cursor = settings->get("cursorBlink").toBool();
            if (qApp->activeWindow()) {
                qApp->activeWindow()->update();
            }
            if (qApp->activeModalWidget()) {
                qApp->activeModalWidget()->update();
            }
            if (qApp->activePopupWidget()) {
                qApp->activePopupWidget()->update();
            }
        }
        if (key == "cursorBlinkTime") {
            m_blink_cursor_time = settings->get("cursorBlinkTime").toInt();
            qApp->styleHints()->setCursorFlashTime(m_blink_cursor_time);
        }
    });

//    connect(settings, &QGSettings::changed, this, [=](const QString &key) {
//        if (key == "useCustomHighlightColor") {
//            m_use_custom_highlight_color = settings->get("useCustomHighlightColor").toBool();
//        }
//        if (key == "customHighlightColor") {
//            m_custom_highlight_color = QColor(settings->get("customHighlightColor").toString());
//        }
//        if (m_use_custom_highlight_color) {
//            //qApp->setStyle(new ProxyStyle(key));
//            auto pal = QApplication::palette();
//            pal.setColor(QPalette::Active, QPalette::Highlight, m_custom_highlight_color);
//            pal.setColor(QPalette::Inactive, QPalette::Highlight, m_custom_highlight_color);
//            pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::transparent);

//            qApp->setPalette(pal);
//            qApp->paletteChanged(pal);
//        } else {
//            auto pal = qApp->style()->standardPalette();
//            qApp->setPalette(pal);
//            qApp->paletteChanged(pal);
//        }
//    });

    m_blur_helper = new BlurHelper(this);
    m_gesture_helper = new GestureHelper(this);
    m_window_manager = new WindowManager(this);

    if (!baseStyle()->inherits("Qt5UKUIStyle")) {
        m_blur_helper->onBlurEnableChanged(false);
    }

    m_app_style_settings = ApplicationStyleSettings::getInstance();
    connect(m_app_style_settings, &ApplicationStyleSettings::colorStretageChanged, [=](const ApplicationStyleSettings::ColorStretagy &stretagy) {
        /*!
          \todo implemet palette switch.
          */
        switch (stretagy) {
        case ApplicationStyleSettings::System: {
            break;
        }
        case ApplicationStyleSettings::Bright: {
            break;
        }
        case ApplicationStyleSettings::Dark: {
            break;
        }
        default:
            break;
        }
    });

    if (QGSettings::isSchemaInstalled("org.ukui.peripherals-mouse")) {
        QGSettings *settings = new QGSettings("org.ukui.peripherals-mouse");
        int mouse_double_click_time = settings->get("doubleClick").toInt();
        if (mouse_double_click_time != qApp->doubleClickInterval()) {
            qApp->setDoubleClickInterval(mouse_double_click_time);
        }
        connect(settings, &QGSettings::changed, qApp, [=] (const QString &key) {
            if (key == "doubleClick") {
                int mouse_double_click_time = settings->get("doubleClick").toInt();
                if (mouse_double_click_time != qApp->doubleClickInterval()) {
                    qApp->setDoubleClickInterval(mouse_double_click_time);
                }
            }
        });
    }
}

bool ProxyStyle::eventFilter(QObject *obj, QEvent *e)
{
//    if (e->type() == QEvent::Hide) {
//        qDebug()<<obj->metaObject()->className()<<e->type()<<"=========\n\n\n";
//    } else {
//        qDebug()<<obj->metaObject()->className()<<e->type();
//    }
    return false;
}

int ProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    //FIXME:
    switch (hint) {
    case QStyle::SH_Menu_Scrollable: {
        return 1;
    }
    case QStyle::SH_BlinkCursorWhenTextSelected: {
        return m_blink_cursor;
    }
    default:
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

void ProxyStyle::polish(QWidget *widget)
{
    if (!baseStyle()->inherits("Qt5UKUIStyle"))
        return QProxyStyle::polish(widget);

    QProxyStyle::polish(widget);

    if(!widget)
        return;
    if (qAppName() == "ukui-menu" && !widget->inherits("QMenu")) {
        return;
    }

    m_gesture_helper->registerWidget(widget);

    /*!
      \todo
      register transparent widget to blurhelper with better way.
      for now it will let some transparent widget show in error.
      i have to avoid them by limitting widget's class name,
      but that is no my expected.
      */
    if (widget->testAttribute(Qt::WA_TranslucentBackground) && widget->isTopLevel()) {
        //FIXME:
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
        m_blur_helper->registerWidget(widget);
#endif

        /*
        if (QString(widget->metaObject()->className())=="QMenu" ||
                widget->inherits("Peony::DirectoryViewMenu") ||
                widget->inherits("Peony::DesktopMenu")) {
            m_blur_helper->registerWidget(widget);
        }
        */
    }

    //qDebug()<<widget->metaObject()->className();
    //add exception.

    if (widget->isWindow()) {
        auto var = widget->property("useStyleWindowManager");

        if (var.isNull()) {
            m_window_manager->registerWidget(widget);
        } else {
            if (var.toBool()) {
                m_window_manager->registerWidget(widget);
            }
        }
    }

    widget->installEventFilter(this);
}

void ProxyStyle::unpolish(QWidget *widget)
{
    if (!baseStyle()->inherits("Qt5UKUIStyle"))
        return QProxyStyle::unpolish(widget);

    if (qAppName() == "ukui-menu" && !widget->inherits("QMenu")) {
        return;
    }

    m_gesture_helper->unregisterWidget(widget);

    //return QProxyStyle::unpolish(widget);
    widget->removeEventFilter(this);

    //FIXME:
    if (widget->testAttribute(Qt::WA_TranslucentBackground) && widget->isTopLevel()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
        m_blur_helper->unregisterWidget(widget);
#endif
    }

    if (widget->isWindow()) {
        auto var = widget->property("useStyleWindowManager");

        if (var.isNull()) {
            m_window_manager->unregisterWidget(widget);
        } else {
            if (var.toBool()) {
                m_window_manager->unregisterWidget(widget);
            }
        }
    }

    QProxyStyle::unpolish(widget);
}

void ProxyStyle::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);

//    if (m_use_custom_highlight_color) {
//        pal.setColor(QPalette::Active, QPalette::Highlight, m_custom_highlight_color);
//        pal.setColor(QPalette::Inactive, QPalette::Highlight, m_custom_highlight_color);
//        pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::transparent);
//    }
}
