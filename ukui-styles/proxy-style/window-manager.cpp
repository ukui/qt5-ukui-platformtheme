#include "window-manager.h"

#include <QWidget>
#include <QMouseEvent>

#include <NETWM>
#include <QX11Info>
#include <xcb/xcb.h>

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(100);
    m_timer.setSingleShot(true);
    m_start_point = QPoint(0, 0);
}

void WindowManager::registerWidget(QWidget *w)
{
    w->removeEventFilter(this);
    w->installEventFilter(this);
}

void WindowManager::unregisterWidget(QWidget *w)
{
    w->removeEventFilter(this);
}

bool WindowManager::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseButtonPress: {
        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        if (event->button() == Qt::LeftButton) {
            buttonPresseEvent(obj, event);
            return false;
        }
        return false;
    }
    case QEvent::MouseMove: {
        if (m_is_dragging) {
            if (m_timer.isActive()) {
                return false;
            } else {
                QMouseEvent *event = static_cast<QMouseEvent*>(e);
                //move request
                mouseMoveEvent(obj, event);
            }
        }
        return false;
    }
    case QEvent::MouseButtonRelease: {
        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        mouseReleaseEvent(obj, event);
        return false;
    }
    default:
        return false;
    }
}

void WindowManager::buttonPresseEvent(QObject *obj, QMouseEvent *e)
{
    m_is_dragging = true;
    m_current_obj = obj;
    m_start_point = e->globalPos();
    m_timer.start();
}

void WindowManager::mouseMoveEvent(QObject *obj, QMouseEvent *e)
{
    QWidget *w = qobject_cast<QWidget*>(obj);

    const QPoint native = e->globalPos();

    auto connection = QX11Info::connection();
    xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);
    NETRootInfo(connection, NET::WMMoveResize).moveResizeRequest(w->winId(), native.x(), native.y(), NET::Move);

    m_start_point = e->globalPos();
    m_timer.start();
}

void WindowManager::mouseReleaseEvent(QObject *obj, QMouseEvent *e)
{
    m_is_dragging = false;
    m_current_obj = nullptr;
    m_start_point = QPoint(0, 0);
}
