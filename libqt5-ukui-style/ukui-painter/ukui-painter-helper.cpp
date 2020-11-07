#include "ukui-painter-helper.h"

ButtonAnimator *UKUIPainterHelper::stillAnimation(QObject *parent)
{
    ButtonAnimator *animator = new ButtonAnimator(parent);
    return animator;
}



QRectF UKUIPainterHelper::rectf_ChangeSize(QRectF rect, qreal changeWidth, qreal changeHight)
{
    if (rect.isEmpty())
        return rect;
    QRectF target_rect = rect;
    target_rect.setSize(QSizeF(target_rect.width() + changeWidth, target_rect.height() + changeHight));
    target_rect.moveCenter(rect.center());
    return target_rect;
}



QRect UKUIPainterHelper::rect_ChangeSize(QRect rect, int changeWidth, int changeHight)
{
     return rectf_ChangeSize(QRectF(rect), qreal(changeWidth), qreal(changeHight)).toRect();
}
