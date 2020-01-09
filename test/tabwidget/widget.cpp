#include "widget.h"
#include <QPushButton>

#include <QListView>
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>

#include <QHBoxLayout>

Widget::Widget(QWidget *parent)
    : QTabWidget(parent)
{
    auto v1 = new QListView(this);
    auto m1 = new QFileSystemModel(v1);
    v1->setModel(m1);
    m1->setRootPath("/");
    v1->setRootIndex(m1->index("/"));
    addTab(v1, "view1");

    auto v2 = new QTreeView;
    v2->setModel(m1);
    v2->setRootIndex(m1->index("/"));
    addTab(v2, "view2");

    addTab(new QPushButton("test1", this), "test1");
    addTab(new QLabel("test2", this), "test2");
    addTab(new QPushButton("test3", this), "test3");
    addTab(new QLabel("test4", this), "test4");
}

Widget::~Widget()
{

}
