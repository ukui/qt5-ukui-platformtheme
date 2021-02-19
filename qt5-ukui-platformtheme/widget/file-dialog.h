#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <qpa/qplatformdialoghelper.h>

#include <QDialog>
#include <QVariant>

class QDialogButtonBox;

class FileDialog;
class FileDialogBase;
class FileDialogHelper;

class FileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
    FileDialogHelper ();
    ~FileDialogHelper () override;

    void initializeDialog();

    bool defaultNameFilterDisables() const override;
    QUrl directory() const override;
    QList<QUrl> selectedFiles() const override;
    QString selectedMimeTypeFilter() const override;
    void selectMimeTypeFilter(const QString &filter) override;
    QString selectedNameFilter() const override;
    void selectNameFilter(const QString &filter) override;
    void selectFile(const QUrl &filename) override;
    void setFilter() override;
    void setDirectory(const QUrl &directory) override;
    bool isSupportedUrl(const QUrl& url) const override;

    void exec() override;
    void hide() override;
    bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;

    QVariant styleHint(StyleHint hint) const override;

private Q_SLOTS:
    void saveSize();

private:
    void restoreSize();

private:
    FileDialogBase              *mDialog = nullptr;
    bool                        mDirectorySet = false;
    bool                        mFileSelected = false;
    bool                        mDialogInitialized = false;

};

class FileDialogBase : public QDialog
{
    Q_OBJECT
    friend class FileDialogHelper;
public:
    explicit FileDialogBase ();

    virtual QUrl directory() = 0;
    virtual void selectMimeTypeFilter(const QString &filter) = 0;
    virtual void selectNameFilter(const QString &filter) = 0;
    virtual void setDirectory(const QUrl &directory) = 0;
    virtual void selectFile(const QUrl &filename) = 0;
    virtual QString selectedMimeTypeFilter() = 0;
    virtual QString selectedNameFilter() = 0;
    virtual QString currentFilterText() = 0;
    virtual QList<QUrl> selectedFiles() = 0;

Q_SIGNALS:
    void closed();
    void fileSelected(const QUrl &file);
    void filesSelected(const QList<QUrl> &files);
    void currentChanged(const QUrl &path);
    void directoryEntered(const QUrl &directory);
    void filterSelected(const QString &filter);

protected:
    void closeEvent(QCloseEvent *e) override;

protected:
    QDialogButtonBox*       mButtons = nullptr;

};

class FileDialog : public FileDialogBase
{
    Q_OBJECT
public:
    friend class FileDialogHelper;

    explicit FileDialog ();
    QUrl directory () override;
    void selectMimeTypeFilter (const QString &filter) override;
    void selectNameFilter (const QString &filter) override;
    void setDirectory (const QUrl &directory) override;
    void selectFile (const QUrl &filename) override;
    void setViewMode (QFileDialogOptions::ViewMode view);
    void setFileMode (QFileDialogOptions::FileMode mode);
    void setCustomLabel(QFileDialogOptions::DialogLabel label, const QString & text);
    QString selectedMimeTypeFilter () override;
    QString selectedNameFilter () override;
    QString currentFilterText () override;
    QList<QUrl> selectedFiles () override;

protected:
//    KFileWidget *const              mFileWidget;              // 主要界面
};

#endif // FILEDIALOG_H
