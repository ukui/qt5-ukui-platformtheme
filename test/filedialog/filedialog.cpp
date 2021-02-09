#include "filedialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>

filedialog::filedialog(QWidget *parent)
    : QWidget(parent)
{
    new QHBoxLayout (this);
    mpb = new QPushButton;
    mpb->setText("文件对话框");
    layout()->addWidget(mpb);

    connect(mpb, &QPushButton::released, this, [=] () {
        QFileDialog fdlg;

        qDebug() << fdlg.getOpenFileUrl().toDisplayString();
    });
}

filedialog::~filedialog()
{
}

