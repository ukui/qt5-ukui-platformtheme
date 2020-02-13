#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QTimer>
#include <QPoint>

class QMouseEvent;

/*!
 * \brief The WindowManager class
 * \details
 * This class is use to help window management.
 * There are some qt's windows, such as frameless window, could not
 * move and move resize by it self. Through registering widget to window
 * manager, we can let them movable and can be maximum/half-maximum like
 * other normal window.
 */
class WindowManager : public QObject
{
    Q_OBJECT
public:
    explicit WindowManager(QObject *parent = nullptr);

    void registerWidget(QWidget *w);
    void unregisterWidget(QWidget *w);

    bool eventFilter(QObject *obj, QEvent *e);

    void buttonPresseEvent(QObject *obj, QMouseEvent *e);
    void mouseMoveEvent(QObject *obj, QMouseEvent *e);
    void mouseReleaseEvent(QObject *obj, QMouseEvent *e);

private:
    QObject *m_current_obj = nullptr;
    bool m_is_dragging = false;
    QTimer m_timer;
    QPoint m_start_point;
};

#endif // WINDOWMANAGER_H
