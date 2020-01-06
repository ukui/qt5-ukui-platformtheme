#include "mainwindow.h"
#include <QApplication>

#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QFontDatabase db;
    //qDebug()<<db.families();
    qDebug()<<db.systemFont(QFontDatabase::GeneralFont);

    return a.exec();
}
