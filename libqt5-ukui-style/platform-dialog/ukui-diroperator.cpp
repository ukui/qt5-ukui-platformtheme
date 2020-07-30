#include "ukui-diroperator.h"
#include "ukui-diroperator_p.h"

#include <QList>
#include <QDir>


UKUIDirOperator::UKUIDirOperator(const QUrl &urlName, QWidget *parent)
    :QWidget(parent),
    d(new UKUIDirOperatorPrivate(this))
{
    if (urlName.isEmpty()) { // no dir specified -> current dir
        QString strPath = QDir::currentPath();
        strPath.append(QLatin1Char('/'));
        d->currUrl = QUrl::fromLocalFile(strPath);
    } else {
        d->currUrl = urlName;
        if (d->currUrl.scheme().isEmpty()) {
            d->currUrl.setScheme(QStringLiteral("file"));
        }

        QString path = d->currUrl.path();
        if (!path.endsWith(QLatin1Char('/'))) {
            path.append(QLatin1Char('/')); // make sure we have a trailing slash!
        }
        d->currUrl.setPath(path);
    }

    d->splitter = new QSplitter(this);
    d->splitter->setChildrenCollapsible(false);
    connect(d->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(_u_slotSplitterMoved(int,int)));

    d->mode = UKUIDirOperator::File;
    d->viewKind = UKUIDirOperator::Simple;
}

UKUIDirOperator::~UKUIDirOperator()
{

}




void UKUIDirOperatorPrivate::_u_slotSplitterMoved(int, int)
{
    const QList<int> sizes = splitter->sizes();
    if (sizes.count() == 2) {
        // remember the width of the preview widget (see UKUIDirOperator::resizeEvent())
        previewWidth = sizes[1];
    }
}
