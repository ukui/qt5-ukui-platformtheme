#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_checkBox_toggled(bool checked)
{
    ui->pushButton->setProperty("useIconHighlightEffect", checked);
    ui->pushButton->update();
    ui->toolButton->setProperty("useIconHighlightEffect", checked);
    ui->toolButton->update();
}
