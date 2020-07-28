#include "ukui-platform-dialog.h"

UKUIPlatformFileDialog::UKUIPlatformFileDialog()
{
    m_timer.setInterval(0);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &UKUIPlatformFileDialog::show);
}

void UKUIPlatformFileDialog::delayedShow()
{
    m_timer.start();
}

void UKUIPlatformFileDialog::discardDelayedShow()
{
    // this is used when hide() is called before timer triggers.
    m_timer.stop();
}

void UKUIPlatformFileDialog::closeEvent(QCloseEvent *e)
{
    emit closed();
    QDialog::closeEvent(e);
}
