#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QPushButton>
#include <QWidget>

class filedialog : public QWidget
{
    Q_OBJECT

public:
    filedialog(QWidget *parent = nullptr);
    ~filedialog();

private:
    QPushButton*                mpb;
};
#endif // FILEDIALOG_H
