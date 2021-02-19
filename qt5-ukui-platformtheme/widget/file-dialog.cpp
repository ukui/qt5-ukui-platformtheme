#include "file-dialog.h"

#include <QDebug>
#include <QWidget>
#include <QWindow>
#include <QVBoxLayout>
#include <QDialogButtonBox>


FileDialogBase::FileDialogBase()
{

}

void FileDialogBase::closeEvent(QCloseEvent *e)
{
    Q_EMIT closed();
    QDialog::closeEvent(e);
}

// FIXME://
FileDialog::FileDialog() : FileDialogBase(), mFileWidget(new QWidget)
{
    setLayout(new QVBoxLayout);

    // 过滤方式修改...
    layout()->addWidget(mFileWidget);

    // 添加关闭和确定按钮
//    mButtons = new QDialogButtonBox (this);
//    mButtons->addButton()
    //

    // 添加按钮组 到 widget

    // debug

}

QUrl FileDialog::directory()
{
    // FIXME://
    return QUrl("file:///");
}

void FileDialog::selectMimeTypeFilter(const QString &filter)
{
    // FIXME://
    return;
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
    // FIXME://
    return "*";
}

QString FileDialog::selectedNameFilter()
{
    // FIXME://
    return "*";
}

QString FileDialog::currentFilterText()
{
    // FIXME://
    return "*";
}

QList<QUrl> FileDialog::selectedFiles()
{
    // FIXME://
    return QList<QUrl>();
}

FileDialogHelper::FileDialogHelper() : QPlatformFileDialogHelper(), mDialog(new FileDialog)
{
    qDebug() << "----------------------------";
    connect(mDialog, &FileDialogBase::closed, this, &FileDialogHelper::saveSize);
    connect(mDialog, &QDialog::finished, this, &FileDialogHelper::saveSize);
    connect(mDialog, &FileDialogBase::currentChanged, this, &QPlatformFileDialogHelper::currentChanged);
    connect(mDialog, &FileDialogBase::directoryEntered, this, &QPlatformFileDialogHelper::directoryEntered);
    connect(mDialog, &FileDialogBase::fileSelected, this, &QPlatformFileDialogHelper::fileSelected);
    connect(mDialog, &FileDialogBase::filesSelected, this, &QPlatformFileDialogHelper::filesSelected);
    connect(mDialog, &FileDialogBase::filterSelected, this, &QPlatformFileDialogHelper::filterSelected);
    connect(mDialog, &QDialog::accepted, this, &QPlatformDialogHelper::accept);
    connect(mDialog, &QDialog::rejected, this, &QPlatformDialogHelper::reject);
    qDebug() << "----------------------------";
}

FileDialogHelper::~FileDialogHelper()
{
    saveSize();
    delete mDialog;
}

void FileDialogHelper::initializeDialog()
{
    mDialogInitialized = true;
//    if (options()->testOption(QFileDialogOptions::ShowDirsOnly)) {
//        m_dialog->deleteLater();
//        KDirSelectDialog *dialog = new KDirSelectDialog(options()->initialDirectory());
//        m_dialog = dialog;
//        connect(dialog, &QDialog::accepted, this, &QPlatformDialogHelper::accept);
//        connect(dialog, &QDialog::rejected, this, &QPlatformDialogHelper::reject);
//        if (options()->isLabelExplicitlySet(QFileDialogOptions::Accept)) { // OK button
//            dialog->setOkButtonText(options()->labelText(QFileDialogOptions::Accept));
//        } else if (options()->isLabelExplicitlySet(QFileDialogOptions::Reject)) { // Cancel button
//            dialog->setCancelButtonText(options()->labelText(QFileDialogOptions::Reject));
//        } else if (options()->isLabelExplicitlySet(QFileDialogOptions::LookIn)) { // Location label
//            //Not implemented yet.
//        }

//        if (!options()->windowTitle().isEmpty())
//            m_dialog->setWindowTitle(options()->windowTitle());
//    } else {
//        // needed for accessing m_fileWidget
//        KDEPlatformFileDialog *dialog = qobject_cast<KDEPlatformFileDialog*>(m_dialog);
//        dialog->m_fileWidget->setOperationMode(options()->acceptMode() == QFileDialogOptions::AcceptOpen ? KFileWidget::Opening : KFileWidget::Saving);
//        if (options()->windowTitle().isEmpty()) {
//            dialog->setWindowTitle(options()->acceptMode() == QFileDialogOptions::AcceptOpen ? i18nc("@title:window", "Open File") : i18nc("@title:window", "Save File"));
//        } else {
//            dialog->setWindowTitle(options()->windowTitle());
//        }
//        if (!m_directorySet) {
//            setDirectory(options()->initialDirectory());
//        }
//        //dialog->setViewMode(options()->viewMode()); // don't override our options, fixes remembering the chosen view mode and sizes!
//        dialog->setFileMode(options()->fileMode());

//        // custom labels
//        if (options()->isLabelExplicitlySet(QFileDialogOptions::Accept)) { // OK button
//            dialog->setCustomLabel(QFileDialogOptions::Accept, options()->labelText(QFileDialogOptions::Accept));
//        } else if (options()->isLabelExplicitlySet(QFileDialogOptions::Reject)) { // Cancel button
//            dialog->setCustomLabel(QFileDialogOptions::Reject, options()->labelText(QFileDialogOptions::Reject));
//        } else if (options()->isLabelExplicitlySet(QFileDialogOptions::LookIn)) { // Location label
//            dialog->setCustomLabel(QFileDialogOptions::LookIn, options()->labelText(QFileDialogOptions::LookIn));
//        }

//        const QStringList mimeFilters = options()->mimeTypeFilters();
//        const QStringList nameFilters = options()->nameFilters();
//        if (!mimeFilters.isEmpty()) {
//            QString defaultMimeFilter;
//            if (options()->acceptMode() == QFileDialogOptions::AcceptSave) {
//                defaultMimeFilter = options()->initiallySelectedMimeTypeFilter();
//                if (defaultMimeFilter.isEmpty()) {
//                    defaultMimeFilter = mimeFilters.at(0);
//                }
//            }
//            dialog->m_fileWidget->setMimeFilter(mimeFilters, defaultMimeFilter);

//            if ( mimeFilters.contains( QStringLiteral("inode/directory") ) )
//                dialog->m_fileWidget->setMode( dialog->m_fileWidget->mode() | KFile::Directory );
//        } else if (!nameFilters.isEmpty()) {
//            dialog->m_fileWidget->setFilter(qt2KdeFilter(nameFilters));
//        }

//        if (!options()->initiallySelectedMimeTypeFilter().isEmpty()) {
//            selectMimeTypeFilter(options()->initiallySelectedMimeTypeFilter());
//        } else if (!options()->initiallySelectedNameFilter().isEmpty()) {
//            selectNameFilter(options()->initiallySelectedNameFilter());
//        }

//        // overwrite option
//        if (options()->testOption(QFileDialogOptions::FileDialogOption::DontConfirmOverwrite)) {
//            dialog->m_fileWidget->setConfirmOverwrite(false);
//         } else if (options()->acceptMode() == QFileDialogOptions::AcceptSave) {
//             dialog->m_fileWidget->setConfirmOverwrite(true);
//        }

//        QStringList schemes = options()->supportedSchemes();
//        dialog->m_fileWidget->setSupportedSchemes(schemes);
//    }
}

bool FileDialogHelper::defaultNameFilterDisables() const
{

}

QUrl FileDialogHelper::directory() const
{
    return mDialog->directory();
}

QList<QUrl> FileDialogHelper::selectedFiles() const
{
    return mDialog->selectedFiles();
}

QString FileDialogHelper::selectedMimeTypeFilter() const
{
    return mDialog->selectedMimeTypeFilter();
}

void FileDialogHelper::selectMimeTypeFilter(const QString &filter)
{
    mDialog->selectMimeTypeFilter(filter);
}

QString FileDialogHelper::selectedNameFilter() const
{
    // 名字搜索
}

void FileDialogHelper::selectNameFilter(const QString &filter)
{

}

void FileDialogHelper::selectFile(const QUrl &filename)
{
    mDialog->selectFile(filename);
    mFileSelected = true;
}

void FileDialogHelper::setFilter()
{

}

void FileDialogHelper::setDirectory(const QUrl &directory)
{
    if (!directory.isEmpty()) {
        mDialog->setDirectory(directory);
        mDirectorySet = true;
    }
}

bool FileDialogHelper::isSupportedUrl(const QUrl &url) const
{
    // gvfs 判断
}

void FileDialogHelper::exec()
{
    restoreSize();
    mDialog->exec();
}

void FileDialogHelper::hide()
{
    mDialog->hide();
}

bool FileDialogHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    initializeDialog();
    mDialog->setWindowFlags(windowFlags);
    mDialog->setWindowModality(windowModality);
    restoreSize();
//    mDialog->windowHandle()->setTransientParent(parent);
//    mDialog->show();
    return true;
}

QVariant FileDialogHelper::styleHint(QPlatformDialogHelper::StyleHint hint) const
{
//    if (hint == DialogIsQtWindow) {
//        return true;
//    }

    return QPlatformDialogHelper::styleHint(hint);
}

void FileDialogHelper::saveSize()
{
    // 保存大小
}

void FileDialogHelper::restoreSize()
{
    // 重置大小
}
