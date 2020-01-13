#ifndef UKUITABWIDGETDEFAULTSLIDEANIMATOR_H
#define UKUITABWIDGETDEFAULTSLIDEANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QStackedWidget>
#include "ukui-tabwidget-animator-iface.h"

#include <QPixmap>

namespace UKUI {

namespace TabWidget {

/*!
 * \brief The DefaultSlideAnimator class
 * \details
 * This class is an implement of UKUITabWidgetAnimatorIface.
 */
class DefaultSlideAnimator : public QVariantAnimation, public UKUITabWidgetAnimatorIface
{
    Q_OBJECT
public:
    explicit DefaultSlideAnimator(QObject *parent = nullptr);

    bool bindTabWidget(QTabWidget *w);
    bool unboundTabWidget();
    QWidget *boundedWidget() {return m_bound_widget;}

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

    /*!
     * \brief m_tmp_page
     * \note
     * insert a tmp tab page into tab widget directly is dangerous,
     * because a custom tab widget's page may be desgined different
     * with normal tab page, such as peony-qt's directory view.
     * In that case, it might lead program crashed when
     * application call a custom page but get a tmp page.
     *
     * for those reasons, i use a temporary widgets bound to the
     * stacked widget with qt's parent&child mechanism.
     * It can float on the top layer or hide on the lower layer of stack,
     * but it does not belong to the elements in the stack (no index),
     * which can avoid the above problems.
     *
     * However, this way might be incompatible with other animations.
     * Because it uses a new widget for painting, not relate with orignal
     * page. Another conflict is the oxygen's fade animation might force
     * raise current tab page when it finished. That might cause a incompleted
     * slide animation if slide duration is longer than fade's.
     */
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
