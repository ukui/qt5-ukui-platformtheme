#ifndef UKUITABWIDGETDEFAULTSLIDEANIMATORFACTORY_H
#define UKUITABWIDGETDEFAULTSLIDEANIMATORFACTORY_H

#include <QObject>
#include "ukui-tabwidget-animator-plugin-iface.h"

namespace UKUI {

namespace TabWidget {

/*!
 * \brief The DefaultSlideAnimatorFactory class
 * \details
 * This class is an internal plugin. It provides a default tabwidget
 * switch animation for QTabWidget and its drived class.
 *
 * \note
 * Note that it used in ukui-style, but you can also use its api in other
 * desktop environment.
 */
class DefaultSlideAnimatorFactory : public QObject, public UKUITabWidgetAnimatorPluginIface
{
    Q_OBJECT
public:
    explicit DefaultSlideAnimatorFactory(QObject *parent = nullptr);

    const QString id() {return tr("Default Slide");}
    const QString brief() {return tr("Let tab widget switch with a slide animation.");}

    const QString key() {return "tab_slide";}
    const QString description() {return brief();}
    AnimatorPluginType pluginType() {return TabWidget;}

    const QString inhertKey() {return "QTabWidget";}
    const QStringList excludeKeys() {return QStringList()<<"Peony::DirectoryWidget";}
    bool isParallel() {return false;}

    UKUITabWidgetAnimatorIface *createAnimator();
};

}

}

#endif // UKUITABWIDGETDEFAULTSLIDEANIMATORFACTORY_H
