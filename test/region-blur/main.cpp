#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("ukui");
    MainWindow w;
    w.setWindowTitle("blur tool bar region");
    w.show();

    QMainWindow w2;
    w2.setWindowTitle("whole window blur");
    w2.setAttribute(Qt::WA_TranslucentBackground);
    w2.show();

    return a.exec();
}
