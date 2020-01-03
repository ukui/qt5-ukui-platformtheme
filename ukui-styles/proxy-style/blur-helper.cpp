#include "blur-helper.h"
#include <QWidget>
#include <KWindowEffects>
#include <QGSettings>
#include <QVariant>
#include <QEvent>

#include <QDebug>

BlurHelper::BlurHelper(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = new QGSettings("org.ukui.style", "/org/ukui/style/", this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "enabled-blur") {
                bool enable = settings->get("enable-blur").toBool();
                this->onBlurEnableChanged(enable);
            }
        });
    }
}

bool BlurHelper::eventFilter(QObject *obj, QEvent *e)
{
    //FIXME:
    //qDebug()<<e<<obj;
    //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
    switch (e->type()) {
        case QEvent::Hide:
        case QEvent::Show:
        case QEvent::Resize:
        {
            // cast to widget and check
            QWidget* widget(qobject_cast<QWidget*>(obj));

            if (!widget)
                break;

            QVariant regionValue = widget->property("blurRegion");
            QRegion region = qvariant_cast<QRegion>(regionValue);
            qDebug()<<regionValue<<region;
            qDebug()<<widget->metaObject()->className()<<widget->geometry()<<widget->mask();
            if (!region.isEmpty()) {
                qDebug()<<"blur region"<<region;
                KWindowEffects::enableBlurBehind(widget->winId(), true, region);
                widget->update();
            } else {
                KWindowEffects::enableBlurBehind(widget->winId(), true);
            }

            //NOTE: we can not setAttribute Qt::WA_TranslucentBackground here,
            //because the window is about to be shown.
            //widget->setAttribute(Qt::WA_TranslucentBackground);
            //KWindowEffects::enableBlurBehind(widget->winId(), true);
            //widget->update();
            break;
        }

        default: break;
    }
    return false;
}

void BlurHelper::registerWidget(QWidget *widget)
{
    if (!m_blur_widgets.contains(widget)) {
        m_blur_widgets<<widget;
        //qDebug()<<KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind);
        KWindowEffects::enableBlurBehind(widget->winId(), true);
        connect(widget, &QWidget::destroyed, this, [=](){
            this->onWidgetDestroyed(widget);
        });
    }
    widget->removeEventFilter(this);
    widget->installEventFilter(this);

    widget->update();
}

void BlurHelper::unregisterWidget(QWidget *widget)
{
    m_blur_widgets.removeOne(widget);
    widget->removeEventFilter(this);
    KWindowEffects::enableBlurBehind(widget->effectiveWinId(), false);
}

void BlurHelper::onBlurEnableChanged(bool enable)
{
    for (auto widget : m_blur_widgets) {
        KWindowEffects::enableBlurBehind(widget->effectiveWinId(), enable);
        if (widget->isVisible())
            widget->update();
    }
}

void BlurHelper::onWidgetDestroyed(QWidget *widget)
{
    unregisterWidget(widget);
}
