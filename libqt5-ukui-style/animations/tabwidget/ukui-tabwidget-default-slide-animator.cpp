#include "ukui-tabwidget-default-slide-animator.h"

#include <QTabWidget>
#include <QStackedWidget>

#include <QEvent>
#include <QChildEvent>

#include <QPainter>

#include <QDebug>

using namespace UKUI::TabWidget;

DefaultSlideAnimator::DefaultSlideAnimator(QObject *parent) : QVariantAnimation (parent)
{
    setDuration(150);
    setStartValue(0.0);
    setEndValue(1.0);
}

bool DefaultSlideAnimator::bindTabWidget(QTabWidget *w)
{
    if (w) {
        m_bound_widget = w;

        for (auto child : w->children()) {
            if (child->objectName() == "qt_tabwidget_stackedwidget") {
                QStackedWidget *stack = qobject_cast<QStackedWidget *>(child);
                for (int i = 0; i < stack->count(); i++) {
                    watchSubPage(stack->widget(i));
                    m_children<<stack->widget(i);
                }

                //FIXME: listen child widget add/remove
                child->installEventFilter(this);

                break;
            }
        }

        m_window_hidden = w->isVisible();
        return true;
    }
    return false;
}

bool DefaultSlideAnimator::unboundTabWidget()
{
    if (m_bound_widget) {
        m_bound_widget->removeEventFilter(this);
        for (auto child : m_bound_widget->children()) {
            child->removeEventFilter(this);
        }
        return true;
    }
    return false;
}

bool DefaultSlideAnimator::eventFilter(QObject *obj, QEvent *e)
{
    //qDebug()<<"event"<<obj<<e->type();
    if (obj->objectName() == "qt_tabwidget_stackedwidget") {
        switch (e->type()) {
        case QEvent::ChildAdded:
        case QEvent::ChildRemoved: {
            auto event = static_cast<QChildEvent *>(e);
            qDebug()<<"is added"<<event->added()<<event->child();
            //refresh?
            //dynamicly add/remove?
            this->unboundTabWidget();
            this->bindTabWidget(m_bound_widget);
            break;
        }
        case QEvent::Hide: {
            m_window_hidden = true;
            break;
        }
        case QEvent::Show: {
            m_window_hidden = false;
            break;
        }
        default:
            return false;
        }
    }

    switch (e->type()) {
    case QEvent::Hide: {
        this->stop();

        QWidget *w = qobject_cast<QWidget *>(obj);
        m_previous_pixmap = w->grab();
        break;
    }
    case QEvent::Show: {
        //if (m_window_hidden)
        //    break;

        if (m_previous_pixmap.isNull()) {
            break;
        }

        QWidget *w = qobject_cast<QWidget *>(obj);
        if (w->size() != m_previous_pixmap.size()) {
            w->resize(m_previous_pixmap.size());
        }
        m_next_pixmap = w->grab();

        qDebug()<<"start animate";
        this->start();
        w->update();
        break;
    }
    case QEvent::Paint: {
        //qDebug()<<"paint";
        QWidget *w = qobject_cast<QWidget *>(obj);
        if (this->state() == QAbstractAnimation::Running) {
            //qDebug()<<this->state()<<"paint";
            QPainter p(w);
            auto value = this->currentValue().toDouble();
            p.setRenderHints(QPainter::Antialiasing);

            //do a horizon slide.

            auto prevSrcRect = QRectF(m_previous_pixmap.rect());
            auto prevTargetRect = QRectF(m_previous_pixmap.rect());
            prevSrcRect.setX(m_previous_pixmap.width()*value);
            prevSrcRect.setWidth(m_previous_pixmap.width() * (1 - value));
            prevTargetRect.setX(0);
            prevTargetRect.setWidth(m_previous_pixmap.width() * (1 - value));
            p.drawPixmap(prevTargetRect, m_previous_pixmap, prevSrcRect);

            auto nextSrcRect = QRectF(m_next_pixmap.rect());
            auto nextTargetRect = QRectF(m_next_pixmap.rect());
            nextSrcRect.setWidth(m_next_pixmap.width() * value);
            nextTargetRect.setX(m_next_pixmap.width() * (1 - value));
            nextTargetRect.setWidth(m_next_pixmap.width() * value);
            p.drawPixmap(nextTargetRect, m_next_pixmap, nextSrcRect);

            //continue paint until animate finished.
            w->update();

            //eat event so that widget will not paint default items and override
            //our custom pixmap.
            return true;
        }
        break;
    }
    case QEvent::Resize: {
        this->stop();
        break;
    }
    default:
        break;
    }

    return false;
}

void DefaultSlideAnimator::watchSubPage(QWidget *w)
{
    w->installEventFilter(this);
}
