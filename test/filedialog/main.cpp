#include "filedialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    filedialog w;
    w.show();
    return a.exec();
}
