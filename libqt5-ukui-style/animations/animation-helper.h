#ifndef ANIMATIONHELPER_H
#define ANIMATIONHELPER_H

#include <QObject>

class QWidget;
class AnimatorIface;

class AnimationHelper : public QObject
{
    Q_OBJECT
public:
    explicit AnimationHelper(QObject *parent = nullptr);
    virtual ~AnimationHelper();

signals:

public slots:
    virtual bool registerWidget(QWidget *) {return false;}
    virtual bool unregisterWidget(QWidget *) {return false;}

protected:
    QHash<QWidget *, AnimatorIface*> *m_animators = nullptr;
};

#endif // ANIMATIONHELPER_H
