#ifndef UKUITABWIDGETDEFAULTSLIDEANIMATOR_H
#define UKUITABWIDGETDEFAULTSLIDEANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QStackedWidget>
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

    bool filterTabWidget(QObject *obj, QEvent *e);
    bool filterStackedWidget(QObject *obj, QEvent *e);
    bool filterSubPage(QObject *obj, QEvent *e);
    bool filterTmpPage(QObject *obj, QEvent *e);

    void clearPixmap();

private:
    QTabWidget *m_bound_widget = nullptr;
    QStackedWidget *m_stack = nullptr;
    QList<QWidget *> m_children;

    QPixmap m_previous_pixmap;
    QPixmap m_next_pixmap;

    QWidget *m_tmp_page = nullptr;

    /*!
     * \brief m_tab_resizing
     * \details
     * If a went to a resize event, the animation should handle
     * widget's relayout after resized.
     * This bool varient is used to help judege if animator's pixmaps
     * and template widget' states should be updated.
     */
    bool m_tab_resizing = false;
};

}

}

#endif // UKUITABWIDGETDEFAULTSLIDEANIMATOR_H
