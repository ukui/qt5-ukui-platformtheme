#include <KWindowEffects>
#include <QApplication>

#include "qt5-ukui-style.h"
#include "ukui-style-settings.h"
#include "black-list.h"
#include "kabstract-style-parameters.h"


extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

void Qt5UKUIStyle::drawMenuPrimitive(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    int rander = 5;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(option->rect.adjusted(+rander, +rander, -rander, -rander), sp->Menu_Radius, sp->Menu_Radius);

    // Draw a black floor
    QPixmap pixmap(option->rect.size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // Blur the black background
    QImage img = pixmap.toImage();
    qt_blurImage(img, 4, false, false);

    // Dig out the center part
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // Shadow rendering
    painter->drawPixmap(option->rect, pixmap, pixmap.rect());

    //That's when I started drawing the frame floor
    QStyleOption opt = *option;
    auto color = opt.palette.color(QPalette::Base);
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        color.setAlphaF(opacity);
    }

    if (qApp->property("blurEnable").isValid()) {
        bool blurEnable = qApp->property("blurEnable").toBool();
        if (!blurEnable) {
            color.setAlphaF(1);
        }
    }

    //if blur effect is not supported, do not use transparent color.
    if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) || blackAppListWithBlurHelper().contains(qAppName())) {
        color.setAlphaF(1);
    }

    opt.palette.setColor(QPalette::Base, color);

    QPen pen(opt.palette.color(QPalette::Normal, QPalette::Dark), 1);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(Qt::transparent);
    painter->setBrush(color);

    QPainterPath path;
    QRegion region;
    if (widget)
        region = widget->mask();
    if (region.isEmpty())
        path.addRoundedRect(opt.rect.adjusted(+rander, +rander, -rander,-rander), sp->Menu_Radius, sp->Menu_Radius);
    else
        path.addRegion(region);

    //painter->drawPolygon(path.toFillPolygon().toPolygon());
    painter->drawPath(path);
    painter->restore();
    return;
}
