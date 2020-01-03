#ifndef BLURHELPER_H
#define BLURHELPER_H

#include <QObject>

class BlurHelper : public QObject
{
    Q_OBJECT
public:
    explicit BlurHelper(QObject *parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *e);
    void registerWidget(QWidget *widget);
    void unregisterWidget(QWidget *widget);

signals:

public slots:
    void onBlurEnableChanged(bool enable);
    void onWidgetDestroyed(QWidget *widget);

private:
    QList<QWidget*> m_blur_widgets;
};

#endif // BLURHELPER_H
