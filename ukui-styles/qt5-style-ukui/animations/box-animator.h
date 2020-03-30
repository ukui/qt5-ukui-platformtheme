#ifndef BOXANIMATOR_H
#define BOXANIMATOR_H

#include <QObject>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>

#include "animator-iface.h"

namespace UKUI {

namespace Box {
class BoxAnimator : public QParallelAnimationGroup, public AnimatorIface
{
    Q_OBJECT
public:
    explicit BoxAnimator(QObject *parent = nullptr);

    bool bindWidget(QWidget *w);
    bool unboundWidget();
    QWidget *boundedWidget() {return m_widget;}
    QVariant value(const QString &property);
    bool setAnimatorStartValue(const QString &property, const QVariant &value);
    bool setAnimatorEndValue(const QString &property, const QVariant &value);
    bool setAnimatorDuration(const QString &property, int duration);
    void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true);
    bool isRunning(const QString &property = nullptr);
    void startAnimator(const QString &property = nullptr);
    void stopAnimator(const QString &property = nullptr);
    int currentAnimatorTime(const QString &property = nullptr);
    int totalAnimationDuration(const QString &property = nullptr);

signals:

private:
    QWidget* m_widget = nullptr;
    QVariantAnimation *m_mouseover;
    QVariantAnimation *m_sunken;
};
}
}

#endif // BOXANIMATOR_H
