#include "ukui-diroperator.h"
#include "ukui-diroperator_p.h"

#include <QList>
#include <QDir>
#include <QProgressBar>
#include <QTimer>
#include <QTreeView>
#include <QHeaderView>
#include <QAction>
#include <QMenu>



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

    setLayoutDirection(Qt::LeftToRight);
    //setDirLister(new KDirLister());
    d->completeListDirty = false;
    connect(&d->completion, SIGNAL(&Peony::PathCompleter::activated(QString)), this, SLOT(&UKUIDirOperator::slotCompletionPath(QString)));//???

    //进度条progressBar
    d->progressBar = new QProgressBar(this);
    d->progressBar->setObjectName(QStringLiteral("d->progressBar"));
    d->progressBar->adjustSize();
    d->progressBar->move(2, height() - d->progressBar->height() - 2);

    d->progressDelayTimer = new QTimer(this);
    d->progressDelayTimer->setObjectName(QStringLiteral("d->progressBar delay timer"));
    connect(d->progressDelayTimer, SIGNAL(timeout()), SLOT(_u_slotShowProgress()));

    setupActions();
    setupMenu();

    d->sorting = QDir::NoSort;  //so updateSorting() doesn't think nothing has changed
    d->updateSorting(QDir::Name | QDir::DirsFirst);

    setFocusPolicy(Qt::WheelFocus);
    setAcceptDrops(true);
}

UKUIDirOperator::~UKUIDirOperator()
{
    delete d;
}

void UKUIDirOperator::setCurrentItem(const QUrl &url)
{

}



void UKUIDirOperator::setupActions()
{

}

void UKUIDirOperator::updateSortActions()
{
    QAction *ascending = d->actionCollection->action(QStringLiteral("ascending"));
    QAction *descending = d->actionCollection->action(QStringLiteral("descending"));

    if ((d->sorting & QDir::Time) == QDir::Time) {
        d->actionCollection->action(QStringLiteral("by name"))->setChecked(true);
        descending->setText(tr("Sort descending", "Z-A"));
        ascending->setText(tr("Sort ascending", "A-Z"));
    } else if ((d->sorting & QDir::Time) == QDir::Time) {
        d->actionCollection->action(QStringLiteral("by date"))->setChecked(true);
        descending->setText(tr("Sort descending", "Newest First"));
        ascending->setText(tr("Sort ascending", "Oldest First"));
    } else if ((d->sorting & QDir::Size) == QDir::Size) {
        d->actionCollection->action(QStringLiteral("by size"))->setChecked(true);
        descending->setText(tr("Sort descending", "Largest First"));
        ascending->setText(tr("Sort ascending", "Smallest First"));
    } else if ((d->sorting & QDir::Type) == QDir::Type) {
        d->actionCollection->action(QStringLiteral("by type"))->setChecked(true);
        descending->setText(tr("Sort descending", "Z-A"));
        ascending->setText(tr("Sort ascending", "A-Z"));
    }
    ascending->setChecked(!(d->sorting & QDir::Reversed));
    descending->setChecked(d->sorting & QDir::Reversed);
    d->actionCollection->action(QStringLiteral("dirs first"))->setChecked(d->sorting & QDir::DirsFirst);
}

void UKUIDirOperator::setupMenu()
{
    QWidgetAction *sortMenu = static_cast<QWidgetAction *>(d->actionCollection->action(QStringLiteral("sorting menu")));
    sortMenu->setProperty("isShortcutConfigurable", false);
    QMenu *sortMenu_menu = new QMenu();
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("by name")));
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("by size")));
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("by date")));
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("by type")));
    sortMenu_menu->addSeparator();
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("ascending")));
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("descending")));
    sortMenu_menu->addSeparator();
    sortMenu_menu->addAction(d->actionCollection->action(QStringLiteral("dirs first")));
    sortMenu->setMenu(sortMenu_menu);

    QMenu *actionMenu_menu = new QMenu();
    actionMenu_menu->addAction(sortMenu);
    actionMenu_menu->addSeparator();
    actionMenu_menu->addAction(d->actionCollection->action(QStringLiteral("view menu")));
    actionMenu_menu->addAction(d->actionCollection->action(QStringLiteral("reload")));
    actionMenu_menu->addSeparator();
    actionMenu_menu->addAction(d->actionCollection->action(QStringLiteral("file manager")));
    actionMenu_menu->addAction(d->actionCollection->action(QStringLiteral("properties")));
    d->actionMenu->setMenu(actionMenu_menu);
}



void UKUIDirOperator::slotCompletionPath(const QString &path)
{
    QUrl url(path);
    if (url.isRelative())
        url = d->currUrl.resolved(url);
    setCurrentItem(url);
}



int UKUIDirOperatorPrivate::sortColumn() const
{
    int column = Peony::FileItemModel::FileName;
    if ((sorting & QDir::Time) == QDir::Time) {
        column = Peony::FileItemModel::ModifiedDate;
    } else if ((sorting & QDir::Size) == QDir::Size) {
        column = Peony::FileItemModel::FileSize;
    } else if ((sorting & QDir::Type) == QDir::Type) {
        column = Peony::FileItemModel::FileType;
    } else {
        Q_ASSERT((sorting & QDir::Time) != QDir::Time && (sorting & QDir::Size) != QDir::Size && (sorting & QDir::Type) != QDir::Type);
    }
    return column;
}

Qt::SortOrder UKUIDirOperatorPrivate::sortOrder() const
{
    return (sorting & QDir::Reversed) ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void UKUIDirOperatorPrivate::updateSorting(QDir::SortFlags sort)
{
    // qDebug() << "changing sort flags from"  << sorting << "to" << sort;
    if (sort == sorting) {
        return;
    }

    if ((sorting ^ sort) & QDir::DirsFirst) {
        // The "Folders First" setting has been changed.
        // We need to make sure that the files and folders are really re-sorted.
        // Without the following intermediate "fake resorting",
        // QSortFilterProxyModel::sort(int column, Qt::SortOrder order)
        // would do nothing because neither the column nor the sort order have been changed.
        Qt::SortOrder tmpSortOrder = (sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
        SortFilter->sort(sortOrder(), tmpSortOrder);
        SortFilter->setSortFoldersFirst(sort & QDir::DirsFirst);
    }

    sorting = sort;
    q->updateSortActions();
    SortFilter->sort(sortColumn(), sortOrder());

    // TODO: The headers from QTreeView don't take care about a sorting
    // change of the proxy model hence they must be updated the manually.
    // This is done here by a qobject_cast, but it would be nicer to:
    // - provide a signal 'sortingChanged()'
    // - connect KDirOperatorDetailView() with this signal and update the
    //   header internally
    QTreeView *treeView = qobject_cast<QTreeView *>(itemView);
    if (treeView != nullptr) {
        QHeaderView *headerView = treeView->header();
        headerView->blockSignals(true);
        headerView->setSortIndicator(sortColumn(), sortOrder());
        headerView->blockSignals(false);
    }
    _u_assureVisibleSelection();
}



void UKUIDirOperatorPrivate::_u_slotSplitterMoved(int, int)
{
    const QList<int> sizes = splitter->sizes();
    if (sizes.count() == 2) {
        // remember the width of the preview widget (see UKUIDirOperator::resizeEvent())
        previewWidth = sizes[1];
    }
}

void UKUIDirOperatorPrivate::_u_slotShowProgress()
{
    progressBar->raise();
    progressBar->show();
}

void UKUIDirOperatorPrivate::_u_assureVisibleSelection()
{
    if (itemView == nullptr) {
        return;
    }
    QItemSelectionModel *selModel = itemView->selectionModel();
    if (selModel->hasSelection()) {
        const QModelIndex index = selModel->currentIndex();
        itemView->scrollTo(index, QAbstractItemView::EnsureVisible);
        //_k_triggerPreview(index);
    }
}
