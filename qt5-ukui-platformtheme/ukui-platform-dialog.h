#ifndef UKUIPLATFORMDIALOG_H
#define UKUIPLATFORMDIALOG_H

#include <QObject>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTimer>

class UKUIPlatformFileDialog : public QDialog
{
    Q_OBJECT
public:
    UKUIPlatformFileDialog();

    void delayedShow();
    void discardDelayedShow();

Q_SIGNALS:
    void closed();

protected:
    void closeEvent(QCloseEvent *e) override;
    QDialogButtonBox *m_buttons;
    QTimer m_timer;
};

#endif // UKUIPLATFORMDIALOG_H
