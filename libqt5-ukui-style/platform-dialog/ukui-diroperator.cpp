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
#include <QIcon>
#include <QList>



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
    d->actionCollection = new UKUIActionCollection(this);
    d->actionCollection->setObjectName(QStringLiteral("UKUIDirOperator::actionCollection"));

    d->actionMenu = new QWidgetAction(this);
    d->actionMenu->setText("Menu");
    d->actionMenu->setProperty("isShortcutConfigurable", false);
    d->actionCollection->addAction(QStringLiteral("popupMenu"), d->actionMenu);

    QAction *upAction = new QAction(QIcon::fromTheme(QStringLiteral("parent-folder")), tr("Parent Folder"), this);
    d->actionCollection->addAction(QStringLiteral("up"), upAction);
    QAction *backAction = new QAction(QIcon::fromTheme(QStringLiteral("go-next")), tr("go back, &Back"), this);
    backAction->setShortcut(Qt::Key_Backspace);
    d->actionCollection->addAction(QStringLiteral("back"), backAction);
    QAction *forwardAction = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")), tr("go forward, &Forward"), this);
    d->actionCollection->addAction(QStringLiteral("forward"), forwardAction);
    QAction *reloadAction = new QAction(QIcon::fromTheme(QStringLiteral("reload")), tr("Reload"), this);
    d->actionCollection->addAction(QStringLiteral("reload"), reloadAction);

    QAction *rename = new QAction(tr("Rename"), this);
    d->actionCollection->addAction(QStringLiteral("rename"), rename);
    QAction *trash = new QAction(QIcon::fromTheme(QStringLiteral("user-trash")), tr("Move to Trash"), this);
    trash->setShortcut(Qt::Key_Delete);
    d->actionCollection->addAction(QStringLiteral("trash"), trash);
    QAction *Delete = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete"), this);
    Delete->setShortcut(Qt::SHIFT + Qt::Key_Delete);
    d->actionCollection->addAction(QStringLiteral("delete"), Delete);



    QAction *byNameAction = new QAction(tr("Sort by Name"), this);
    byNameAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("by name"), byNameAction);
    QAction *bySizeAction = new QAction(tr("Sort by Size"), this);
    bySizeAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("by size"), bySizeAction);
    QAction *byDateAction = new QAction(tr("Sort by Date"), this);
    byDateAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("by date"), byDateAction);
    QAction *byTypeAction = new QAction(tr("Sort by Type"), this);
    byTypeAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("by type"), byTypeAction);
    QActionGroup *sortGroup = new QActionGroup(this);
    byNameAction->setActionGroup(sortGroup);
    bySizeAction->setActionGroup(sortGroup);
    byDateAction->setActionGroup(sortGroup);
    byTypeAction->setActionGroup(sortGroup);

    QActionGroup *sortOrderGroup = new QActionGroup(this);
    sortOrderGroup->setExclusive(true);
    QAction *ascendingAction = new QAction(tr("Ascending"), this);
    ascendingAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("ascending"), ascendingAction);
    ascendingAction->setActionGroup(sortOrderGroup);
    QAction *descendingAction = new QAction(tr("Descending"), this);
    descendingAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("descending"), descendingAction);
    descendingAction->setActionGroup(sortOrderGroup);
    QAction *dirsFirstAction = new QAction(tr("Folders First"), this);
    dirsFirstAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("dirs first"), dirsFirstAction);

    QWidgetAction *sortMenu = new QWidgetAction(this);
    sortMenu->setText("Sorting");
    sortMenu->setIcon(QIcon::fromTheme(QStringLiteral("view-sort")));
    sortMenu->setProperty("isShortcutConfigurable", false);
    d->actionCollection->addAction(QStringLiteral("sorting menu"), sortMenu);
    QMenu *sortMenu_menu = new QMenu();
    sortMenu_menu->addAction(byNameAction);
    sortMenu_menu->addAction(bySizeAction);
    sortMenu_menu->addAction(byDateAction);
    sortMenu_menu->addAction(byTypeAction);
    sortMenu_menu->addSeparator();
    sortMenu_menu->addAction(ascendingAction);
    sortMenu_menu->addAction(descendingAction);
    sortMenu_menu->addSeparator();
    sortMenu_menu->addAction(dirsFirstAction);
    sortMenu->setMenu(sortMenu_menu);



    QAction *iconsViewAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-icons")), tr("Icons View"), this);
    iconsViewAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("icons view"), iconsViewAction);
    QAction *compactViewAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-details")), tr("Compact View"), this);
    compactViewAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("compact view"), compactViewAction);
    QAction *detailsViewAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), tr("Details View"), this);
    detailsViewAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("details view"), detailsViewAction);

    QActionGroup *viewModeGroup = new QActionGroup(this);
    viewModeGroup->setExclusive(true);
    iconsViewAction->setActionGroup(viewModeGroup);
    compactViewAction->setActionGroup(viewModeGroup);
    detailsViewAction->setActionGroup(viewModeGroup);



    QAction *shortAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-icons")), tr("Short View"), this);
    shortAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("short view"),  shortAction);
    QAction *detailedAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-details")), tr("Detailed View"), this);
    detailedAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("detailed view"), detailedAction);
    QAction *treeAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), tr("Tree View"), this);
    treeAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("tree view"), treeAction);
    QAction *detailedTreeAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-tree")), tr("Detailed Tree View"), this);
    detailedTreeAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("detailed tree view"), detailedTreeAction);

    QActionGroup *viewGroup = new QActionGroup(this);
    shortAction->setActionGroup(viewGroup);
    detailedAction->setActionGroup(viewGroup);
    treeAction->setActionGroup(viewGroup);
    detailedTreeAction->setActionGroup(viewGroup);



    QAction *allowExpansionAction = new QAction("Allow Expansion in Details View", this);
    allowExpansionAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("allow expansion"), allowExpansionAction);
    QAction *showHiddenAction = new QAction("Show Hidden Files", this);
    showHiddenAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("show hidden"), showHiddenAction);
    showHiddenAction->setShortcuts({Qt::ALT + Qt::Key_Period, Qt::CTRL + Qt::Key_H, Qt::Key_F8});
    QAction *showSidebarAction = new QAction("Show Places Panel", this);
    showSidebarAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("toggleSpeedbar"), showSidebarAction);
    showSidebarAction->setShortcut(QKeySequence(Qt::Key_F9));
    QAction *showBookmarksAction = new QAction("Show Bookmarks Button", this);
    showBookmarksAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("toggleBookmarks"), showBookmarksAction);
    QAction *previewAction = new QAction("Show Preview Panel", this);
    previewAction->setCheckable(true);
    d->actionCollection->addAction(QStringLiteral("preview"), previewAction);
    previewAction->setShortcut(Qt::Key_F11);

    QWidgetAction *optionMenu = new QWidgetAction(this);
    optionMenu->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    optionMenu->setText("Options");
    optionMenu->setProperty("isShortcutConfigurable", false);
    d->actionCollection->addAction(QStringLiteral("option menu"), optionMenu);
    QMenu *settings_menu = new QMenu();
    settings_menu->addAction(allowExpansionAction);
    settings_menu->addSeparator();
    settings_menu->addAction(showHiddenAction);
    settings_menu->addAction(showSidebarAction);
    settings_menu->addAction(showBookmarksAction);
    settings_menu->addAction(previewAction);
    optionMenu->setMenu(settings_menu);



    d->actionCollection->addAssociatedWidget(this);
    const QList<QAction *> list = d->actionCollection->allaction();
    for (QAction *action : list)
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
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
