#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegion>

class QListWidget;
class QToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QRegion blurRegion READ blurRegion WRITE setBlurRegion MEMBER m_region)
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    const QRegion blurRegion() {return m_blur_region;}
    void setBlurRegion(const QRegion &region) {m_blur_region = region;}

protected:
    void resizeEvent(QResizeEvent *e);

private:
    QListWidget *m_list_view;
    QToolBar *m_tool_bar;

    QRegion m_blur_region = QRegion();
};

#endif // MAINWINDOW_H
