#include "ukui-painter-helper.h"



bool UKUIPainterHelper::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QAbstractAnimation *animator, const QWidget *widget)
{
    if(const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option))
    {
        const QRect& rect(option->rect);
        const QPalette& palette(option->palette);

    }
    return false;
}



bool UKUIPainterHelper::drawPushButtonLable(const QStyleOption *option, QPainter *painter, const QAbstractAnimation *animator, const QWidget *widget)
{
    return false;
}
