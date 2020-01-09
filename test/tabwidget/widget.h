#ifndef WIDGET_H
#define WIDGET_H

#include <QTabWidget>

class Widget : public QTabWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();
};

#endif // WIDGET_H
