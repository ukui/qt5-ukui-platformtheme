#include "widget.h"
#include <QApplication>

/// make a QTabWidget have a slide animation.
/// This is integrated in ukui style by default.
/// \see
/// Qt5UKUIStyle, UKUI::TabWidget::DefaultSlideAnimator.
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
