#ifndef UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H
#define UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H

#include <QParallelAnimationGroup>
#include "animator-iface.h"

class QVariantAnimation;

namespace UKUI {

namespace ScrollBar {

class DefaultInteractionAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
public:
    explicit DefaultInteractionAnimator(QObject *parent = nullptr);

    bool bindWidget(QWidget *w);
    bool unboundWidget();
    QWidget *boundedWidget() {return m_widget;}

    QVariant value(const QString &property);
    bool isRunning(const QString &property = nullptr);
    bool setAnimatorStartValue(const QString &property, const QVariant &value);
    bool setAnimatorEndValue(const QString &property, const QVariant &value);

    bool setAnimatorDuration(const QString &property, int duration);
    void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true);
    void startAnimator(const QString &property = nullptr);
    void stopAnimator(const QString &property = nullptr);
    int currentAnimatorTime(const QString &property = nullptr);
    int totalAnimationDuration(const QString &property);

private:
    QWidget *m_widget = nullptr;

    QVariantAnimation *m_bg_opacity;
    QVariantAnimation *m_groove_width;
    QVariantAnimation *m_slider_opacity;
};

}

}

#endif // UKUISCROLLBARDEFAULTINTERACTIONANIMATOR_H
