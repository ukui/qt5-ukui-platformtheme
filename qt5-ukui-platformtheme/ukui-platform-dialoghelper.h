#ifndef UKUIPLATFORMDIALOGHELPER_H
#define UKUIPLATFORMDIALOGHELPER_H

#include "ukui-platform-dialog.h"

#include <QObject>
#include <qpa/qplatformdialoghelper.h>
#include <QWindow>

class UKUIPlatformFileDialoghelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
    explicit UKUIPlatformFileDialoghelper();
    ~UKUIPlatformFileDialoghelper() override;

    bool defaultNameFilterDisables() const override;

    QUrl directory() const override;
    void setDirectory(const QUrl &directory) override;

    void selectFile(const QUrl &filename) override;
    QList<QUrl> selectedFiles() const override;

    void selectMimeTypeFilter(const QString &filter) override;
    QString selectedMimeTypeFilter() const override;

    void selectNameFilter(const QString &filter) override;
    QString selectedNameFilter() const override;

    void setFilter() override;

    bool isSupportedUrl(const QUrl& url) const override;

    void exec() override;
    void hide() override;
    bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;


private:
    void restoreSize();
    UKUIPlatformFileDialog *m_dialog;
    bool m_directorySet = false;
    bool m_fileSelected = false;
    bool m_dialogInitialized = false;
};

#endif // UKUIPLATFORMDIALOGHELPER_H
