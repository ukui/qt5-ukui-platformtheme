#ifndef ANIMATORIFACE_H
#define ANIMATORIFACE_H

#include "animator-plugin-iface.h"

class QWidget;

class AnimatorIface
{
public:
    virtual ~AnimatorIface() {}

    virtual bool bindWidget(QWidget *w) = 0;
    virtual bool unboundWidget() = 0;
    virtual QWidget *boundedWidget() = 0;
};

#endif // ANIMATORIFACE_H
