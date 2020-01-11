#ifndef UKUITABWIDGETANIMATORIFACE_H
#define UKUITABWIDGETANIMATORIFACE_H

#include <QTabWidget>

/*!
 * \brief The UKUITabWidgetAnimatorIface class
 * \details
 * This class define the interface for doing a QTabWidget's animation.
 * a tabwidget animator should bind only one tabwidget with bindTabWidget(),
 * and can be unbounded with unboundTabWidget().
 *
 * Animator is created by AnimatorPlugin, which is another interface's implement
 * of UKUI style animation's frameworks.
 *
 * \see UKUITabWidgetAnimatorPluginIface
 */
class UKUITabWidgetAnimatorIface
{
public:
    virtual ~UKUITabWidgetAnimatorIface() {}

    /*!
     * \brief bindTabWidget
     * \param w widget should be bound.
     * \return true if successed.
     * \details
     * this method is used for binding a animator instance for a tab widget.
     * You have to implement this function in your own implement class.
     */
    virtual bool bindTabWidget(QTabWidget *w) = 0;

    /*!
     * \brief unboundTabWidget
     * \return true if successed.
     * \details
     * this method is used to unbound the animator instance and tab widget.
     * You have to implement this function in your own implement class.
     */
    virtual bool unboundTabWidget() = 0;
};

#endif // UKUITABWIDGETANIMATORIFACE_H
