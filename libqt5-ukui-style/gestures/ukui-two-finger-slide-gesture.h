#ifndef TWOFINGERSLIDEGESTURE_H
#define TWOFINGERSLIDEGESTURE_H

#include <QGesture>
#include <QGestureRecognizer>

namespace UKUI {

class TwoFingerSlideGesture : public QGesture
{
    friend class TwoFingerSlideGestureRecognizer;
    Q_OBJECT
public:
    enum Direction {
        Invalid,
        Horizal,
        Vertical
    };

    explicit TwoFingerSlideGesture(QObject *parent = nullptr);

    Direction direction() {return m_direction;}
    const QPoint startPos() {return m_start_pos;}
    const QPoint currentPos() {return m_current_pos;}
    int delta();
    int totalDelta();

private:
    QPoint m_start_pos;
    QPoint m_last_pos;
    QPoint m_current_pos;

    Direction m_direction = Invalid;
};

class TwoFingerSlideGestureRecognizer : public QGestureRecognizer
{
public:
    explicit TwoFingerSlideGestureRecognizer();

    QGesture *create(QObject *target) override;
    QGestureRecognizer::Result recognize(QGesture *gesture, QObject *watched, QEvent *event) override;
    void reset(QGesture *gesture) override;
};

}

#endif // TWOFINGERSLIDEGESTURE_H
