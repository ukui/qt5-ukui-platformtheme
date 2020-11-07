#include "ukui-color-helper.h"
#include "button-animator.h"


#include <QStyle>
#include <QStyleOption>
#include <QWidget>
#include <QVariantAnimation>
#include <QPainter>
#include <QRect>
#include <QRectF>


namespace UKUIPainterHelper {
ButtonAnimator *stillAnimation(QObject *parent = nullptr);

bool drawPushButtonBevel(const QStyleOption *option, QPainter *painter,  AnimatorIface *animator = nullptr, const QWidget *widget = nullptr);
bool drawPushButtonLable(const QStyleOption *option, QPainter *painter,  const QStyle *style = nullptr, const QWidget *widget = nullptr);

QRectF rectf_ChangeSize(QRectF rect, qreal changeWidth = 0.0, qreal changeHight = 0.0);
QRect rect_ChangeSize(QRect rect, int changeWidth = 0, int changeHight = 0);

}
