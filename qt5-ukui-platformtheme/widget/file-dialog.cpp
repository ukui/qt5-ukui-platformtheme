#include "file-dialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>


FileDialogBase::FileDialogBase()
{

}

void FileDialogBase::closeEvent(QCloseEvent *e)
{
    Q_EMIT closed();
    QDialog::closeEvent(e);
}

// FIXME://
FileDialog::FileDialog() : FileDialogBase()
{
    setLayout(new QVBoxLayout);

    // 过滤方式修改...
    // 添加主窗口

    // 添加关闭和确定按钮
    mButtons = new QDialogButtonBox (this);
//    mButtons->addButton()
    //

    // 添加按钮组 到 widget

}

QUrl FileDialog::directory()
{

}

void FileDialog::selectMimeTypeFilter(const QString &filter)
{

}

void FileDialog::selectNameFilter(const QString &filter)
{

}

void FileDialog::setDirectory(const QUrl &directory)
{

}

void FileDialog::selectFile(const QUrl &filename)
{

}

void FileDialog::setViewMode(QFileDialogOptions::ViewMode view)
{

}

void FileDialog::setFileMode(QFileDialogOptions::FileMode mode)
{

}

void FileDialog::setCustomLabel(QFileDialogOptions::DialogLabel label, const QString &text)
{

}

QString FileDialog::selectedMimeTypeFilter()
{

}

QString FileDialog::selectedNameFilter()
{

}

QString FileDialog::currentFilterText()
{

}

QList<QUrl> FileDialog::selectedFiles()
{

}

FileDialogHelper::FileDialogHelper()
{

}

FileDialogHelper::~FileDialogHelper()
{

}

void FileDialogHelper::initializeDialog()
{

}

bool FileDialogHelper::defaultNameFilterDisables() const
{

}

QUrl FileDialogHelper::directory() const
{

}

QList<QUrl> FileDialogHelper::selectedFiles() const
{

}

QString FileDialogHelper::selectedMimeTypeFilter() const
{

}

void FileDialogHelper::selectMimeTypeFilter(const QString &filter)
{

}

QString FileDialogHelper::selectedNameFilter() const
{

}

void FileDialogHelper::selectNameFilter(const QString &filter)
{

}

void FileDialogHelper::selectFile(const QUrl &filename)
{

}

void FileDialogHelper::setFilter()
{

}

void FileDialogHelper::setDirectory(const QUrl &directory)
{

}

bool FileDialogHelper::isSupportedUrl(const QUrl &url) const
{

}

void FileDialogHelper::exec()
{

}

void FileDialogHelper::hide()
{

}

bool FileDialogHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{

}

QVariant FileDialogHelper::styleHint(QPlatformDialogHelper::StyleHint hint) const
{

}

void FileDialogHelper::saveSize()
{

}

void FileDialogHelper::restoreSize()
{

}
