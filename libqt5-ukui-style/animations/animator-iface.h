#ifndef ANIMATORIFACE_H
#define ANIMATORIFACE_H

#include "animator-plugin-iface.h"
#include <QVariant>

class QWidget;

class AnimatorIface
{
public:
    virtual ~AnimatorIface() {}

    virtual bool bindWidget(QWidget *w) {return false;}
    virtual bool unboundWidget() {return false;}
    virtual QWidget *boundedWidget() {return nullptr;}

    virtual QVariant value(const QString &property) {return QVariant();}
    virtual bool setAnimatorStartValue(const QString &property, const QVariant &value) {return false;}
    virtual bool setAnimatorEndValue(const QString &property, const QVariant &value) {return false;}
    virtual bool setAnimatorDuration(const QString &property, int duration) {return false;}

    virtual void setAnimatorDirectionForward(const QString &property = nullptr, bool forward = true) {}
    virtual bool isRunning(const QString &property = nullptr) {return false;}
    virtual void startAnimator(const QString &property = nullptr) {}
    virtual void stopAnimator(const QString &property = nullptr) {}
    virtual int currentAnimatorTime(const QString &property = nullptr) {return 0;}
    virtual int toatalDuration(const QString &property = nullptr) {return 0;}
};

#endif // ANIMATORIFACE_H
