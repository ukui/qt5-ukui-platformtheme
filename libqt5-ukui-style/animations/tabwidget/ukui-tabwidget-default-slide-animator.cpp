#include "ukui-tabwidget-default-slide-animator.h"

#include <QTabWidget>
#include <QStackedWidget>

#include <QEvent>
#include <QChildEvent>

#include <QPainter>

#include <QTimer>

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

        //watch tab widget
        w->installEventFilter(this);

        m_tmp_page = new QWidget;
        //watch tmp page;
        m_tmp_page->installEventFilter(this);

        for (auto child : w->children()) {
            if (child->objectName() == "qt_tabwidget_stackedwidget") {
                auto stack = qobject_cast<QStackedWidget *>(child);
                m_stack = stack;
                //watch stack widget
                m_stack->installEventFilter(this);
                stack->addWidget(m_tmp_page);
                break;
            }
        }

        for (int i = 0; i < w->count(); i++) {
            //watch sub page
            watchSubPage(w->widget(i));
        }

        connect(w, &QTabWidget::currentChanged, w, [=](int index){
            qDebug()<<w->currentIndex();
            qDebug()<<index;

            m_next_pixmap = m_bound_widget->currentWidget()->grab();
            m_next_pixmap.save("next.png");
            m_tmp_page->raise();
            m_tmp_page->show();
        });

        //we should wait a while so that we can grab current widget
        //correctly.
        QTimer::singleShot(1, w, [=](){
            m_previous_pixmap = w->currentWidget()->grab();
        });

        return true;
    }
    return false;
}

bool DefaultSlideAnimator::unboundTabWidget()
{
    if (m_bound_widget) {
        for (auto w : m_bound_widget->children()) {
            w->removeEventFilter(this);
        }

        m_tmp_page->removeEventFilter(this);
        m_tmp_page->deleteLater();
        m_tmp_page = nullptr;
        return true;
    }
    return false;
}

bool DefaultSlideAnimator::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_tmp_page) {
        return filterTmpPage(obj, e);
    }
    if (obj == m_stack) {
        return filterStackedWidget(obj, e);
    }
    if (obj == m_bound_widget) {
        return filterTabWidget(obj, e);
    }
    return filterSubPage(obj, e);
}

void DefaultSlideAnimator::watchSubPage(QWidget *w)
{
    w->installEventFilter(this);
}

bool DefaultSlideAnimator::filterTabWidget(QObject *obj, QEvent *e)
{
    return false;
}

bool DefaultSlideAnimator::filterStackedWidget(QObject *obj, QEvent *e)
{
    return false;
}

bool DefaultSlideAnimator::filterSubPage(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Show: {
        this->start();
        return false;
    }
    case QEvent::Hide: {
        if (!m_next_pixmap.isNull())
            m_previous_pixmap = m_next_pixmap;
        this->stop();
        return false;
    }
    case QEvent::Resize: {
        this->stop();
        m_previous_pixmap.detach();
        m_next_pixmap.detach();
        return false;
    }
    default:
        return false;
    }
}

bool DefaultSlideAnimator::filterTmpPage(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Show: {
        return false;
    }
    case QEvent::Paint: {
        QWidget *w = qobject_cast<QWidget *>(obj);
        if (this->state() == QAbstractAnimation::Running) {
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
        qDebug()<<"hide";
        m_tmp_page->hide();
        m_stack->stackUnder(m_tmp_page);
        m_previous_pixmap = m_next_pixmap;
        return false;
    }
    default:
        return false;
    }
}
