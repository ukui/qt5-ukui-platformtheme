#include "mainwindow.h"

#include <QListWidget>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_tool_bar = new QToolBar(this);
    m_tool_bar->addAction("test1");
    m_tool_bar->addAction("test2");
    m_tool_bar->addAction("test3");
    m_tool_bar->addAction("test4");
    addToolBar(m_tool_bar);

    m_list_view = new QListWidget(this);
    m_list_view->setAttribute(Qt::WA_TranslucentBackground);
    m_list_view->setAttribute(Qt::WA_Hover);
    m_list_view->setStyleSheet("background: transparent");
    m_list_view->addItem("test1");
    m_list_view->addItem("test2");
    m_list_view->addItem("test3");
    m_list_view->addItem("test4");
    setCentralWidget(m_list_view);
}

MainWindow::~MainWindow()
{

}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    m_blur_region = m_tool_bar->rect();
}
