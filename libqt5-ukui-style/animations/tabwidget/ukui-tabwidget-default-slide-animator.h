#ifndef UKUITABWIDGETDEFAULTSLIDEANIMATOR_H
#define UKUITABWIDGETDEFAULTSLIDEANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include "ukui-tabwidget-animator-iface.h"

#include <QPixmap>

namespace UKUI {

namespace TabWidget {

class DefaultSlideAnimator : public QVariantAnimation, public UKUITabWidgetAnimatorIface
{
    Q_OBJECT
public:
    explicit DefaultSlideAnimator(QObject *parent = nullptr);

    bool bindTabWidget(QTabWidget *w);
    bool unboundTabWidget();

    bool eventFilter(QObject *obj, QEvent *e);

protected:
    void watchSubPage(QWidget *w);

private:
    QTabWidget *m_bound_widget = nullptr;
    QList<QWidget *> m_children;

    QPixmap m_previous_pixmap;
    QPixmap m_next_pixmap;

    bool m_window_hidden = false;
};

}

}

#endif // UKUITABWIDGETDEFAULTSLIDEANIMATOR_H
