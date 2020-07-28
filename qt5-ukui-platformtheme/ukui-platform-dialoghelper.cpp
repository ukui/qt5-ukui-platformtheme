#include "ukui-platform-dialoghelper.h"

UKUIPlatformFileDialoghelper::UKUIPlatformFileDialoghelper()
    : QPlatformFileDialogHelper()
    , m_dialog(new UKUIPlatformFileDialog)
{

}

UKUIPlatformFileDialoghelper::~UKUIPlatformFileDialoghelper()
{
    delete m_dialog;
}

bool UKUIPlatformFileDialoghelper::defaultNameFilterDisables() const
{

}

QUrl UKUIPlatformFileDialoghelper::directory() const
{

}

void UKUIPlatformFileDialoghelper::setDirectory(const QUrl &directory)
{

}

void UKUIPlatformFileDialoghelper::selectFile(const QUrl &filename)
{

}

QList<QUrl> UKUIPlatformFileDialoghelper::selectedFiles() const
{

}

void UKUIPlatformFileDialoghelper::selectMimeTypeFilter(const QString &filter)
{

}

QString UKUIPlatformFileDialoghelper::selectedMimeTypeFilter() const
{

}

void UKUIPlatformFileDialoghelper::selectNameFilter(const QString &filter)
{

}

QString UKUIPlatformFileDialoghelper::selectedNameFilter() const
{

}

void UKUIPlatformFileDialoghelper::setFilter()
{

}

bool UKUIPlatformFileDialoghelper::isSupportedUrl(const QUrl& url) const
{

}

void UKUIPlatformFileDialoghelper::exec()
{

}

void UKUIPlatformFileDialoghelper::hide()
{

}
bool UKUIPlatformFileDialoghelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{

}
