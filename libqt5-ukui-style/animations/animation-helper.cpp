#include "animation-helper.h"

#include <QWidget>
#include "animator-iface.h"

AnimationHelper::AnimationHelper(QObject *parent) : QObject(parent)
{
    m_animators = new QHash<QWidget *, AnimatorIface *>();
}

AnimationHelper::~AnimationHelper()
{
    for (auto animator : *m_animators) {
        delete animator;
    }
    delete m_animators;
}
