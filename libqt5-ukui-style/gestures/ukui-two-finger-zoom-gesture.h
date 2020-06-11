#ifndef TWOFINGERZOOMGESTURE_H
#define TWOFINGERZOOMGESTURE_H

#include <QGesture>
#include <QGestureRecognizer>

#include <QPair>

namespace UKUI {

class TwoFingerZoomGesture : public QGesture
{
    friend class TwoFingerZoomGestureRecognizer;
    Q_OBJECT
public:
    enum Direction {
        Invaild,
        Unkown,
        ZoomIn,
        ZoomOut
    };

    explicit TwoFingerZoomGesture(QObject *parent = nullptr);

    Direction zoomDirection() {return m_zoom_direction;}
    QPair<QPoint, QPoint> startPoints() {return m_start_points;}
    QPair<QPoint, QPoint> lastPoints() {return m_last_points;}
    QPair<QPoint, QPoint> currentPoints() {return m_current_points;}

private:
    QPair<QPoint, QPoint> m_start_points;
    QPair<QPoint, QPoint> m_last_points;
    QPair<QPoint, QPoint> m_current_points;

    qreal m_start_points_distance = -1;
    qreal m_last_points_distance = -1;

    Direction m_zoom_direction = Invaild;
};

class TwoFingerZoomGestureRecognizer : public QGestureRecognizer
{
public:
    explicit TwoFingerZoomGestureRecognizer();

    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;
    void reset(QGesture *gesture) override;
};

}

#endif // TWOFINGERZOOMGESTURE_H
