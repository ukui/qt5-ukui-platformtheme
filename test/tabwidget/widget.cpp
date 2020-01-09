#include "widget.h"
#include <QPushButton>

Widget::Widget(QWidget *parent)
    : QTabWidget(parent)
{
    addTab(new QPushButton("test1", this), "test1");
    addTab(new QPushButton("test2", this), "test2");
    addTab(new QPushButton("test3", this), "test3");
    addTab(new QPushButton("test4", this), "test4");
}

Widget::~Widget()
{

}
