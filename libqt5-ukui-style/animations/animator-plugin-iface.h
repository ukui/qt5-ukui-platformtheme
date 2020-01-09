#ifndef ANIMATORPLUGINIFACE_H
#define ANIMATORPLUGINIFACE_H

#include <QString>

class UKUIAnimatorPluginIface
{
public:
    enum AnimatorPluginType {
        TabWidget,
        PushButton,
        ItemView,
        Other
    };

    virtual ~UKUIAnimatorPluginIface() {}

    virtual const QString id() = 0;
    virtual const QString brief() = 0;

    /*!
     * \brief inhertKey
     * \return
     * \details
     * When a style polish a widget, ukui animation frameworks will bind the animator
     * to the widget. This value is the keyword for judgeing if the widget should be bound
     * with animator. For example, return this value with "QWidget", then all the widgets
     * will be bound.
     */
    virtual const QString inhertKey() = 0;

    /*!
     * \brief excludeKeys
     * \return
     * \details
     * In contrast to inhertKey(), this list is a "blacklist" of
     * widgets should not be bound with the animator.
     */
    virtual const QStringList excludeKeys() = 0;

    /*!
     * \brief isParallel
     * \return
     * \details
     * Indicate if the animator which plugin created is compatible with other
     * animators.
     *
     * \note
     * This variable has no practical effect,
     * but we hope that the animations acting on the same control
     * can be parallelized, although it is difficult to achieve.
     *
     * If you note the animator is not parallelized, other animator
     * might be invalid. For example, the default tabwidget slide animator
     * will hijack the paint event of current tabwidget's tab. This might
     * let other animator can not do a paint in current tab.
     */
    virtual bool isParallel() = 0;
};

#endif // ANIMATORPLUGINIFACE_H
