#include <QStyleOption>
#include <QWidget>
#include <QAbstractAnimation>

namespace UKUIPainterHelper {
bool drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QAbstractAnimation *animator = nullptr, const QWidget *widget = nullptr);
bool drawPushButtonLable(const QStyleOption *option, QPainter *painter, const QAbstractAnimation *animator = nullptr, const QWidget *widget = nullptr);
}
