#ifndef UKUIDIROPERATOR_P_H
#define UKUIDIROPERATOR_P_H

#include "ukui-diroperator.h"
#include "filedialog-sort-filter-proxymodel.h"
#include "ukui-action-collection.h"

#include <PeonyPathCompleter>
#include <PeonyFileItemModel>
#include <QSplitter>
#include <QUrl>
#include <QProgressBar>
#include <QTimer>
#include <QDir>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QWidgetAction>



class UKUIDirOperatorPrivate
{
public:
    explicit UKUIDirOperatorPrivate(UKUIDirOperator *parent)
        : q(parent),
          dirModel(nullptr),
          SortFilter(nullptr),
          splitter(nullptr),
          progressBar(nullptr),
          previewWidth(0),
          progressDelayTimer(nullptr),
          actionCollection(nullptr)
    {

    }

    ~UKUIDirOperatorPrivate()
    {
        delete SortFilter;
        SortFilter = nullptr;
        delete dirModel;
        dirModel = nullptr;
        delete progressDelayTimer;
        progressDelayTimer = nullptr;
    }

    UKUIDirOperator * const q;

    QAbstractItemView *itemView;
    Peony::FileItemModel *dirModel;
    FileDialogSortFilterProxyModel *SortFilter;

    QUrl currUrl;

    Peony::PathCompleter completion;
    Peony::PathCompleter dirCompletion;

    QDir::SortFlags sorting;
    QSplitter *splitter;
    UKUIDirOperator::Mode mode;
    QProgressBar *progressBar;
    UKUIDirOperator::FileView viewKind;
    int previewWidth;
    QTimer *progressDelayTimer;

    QWidgetAction *actionMenu;
    QWidgetAction *decorationMenu;
    UKUIActionCollection *actionCollection;

    bool completeListDirty;



    int sortColumn() const;
    Qt::SortOrder sortOrder() const;
    void updateSorting(QDir::SortFlags sort);



    void _u_slotSplitterMoved(int, int);
    void _u_slotShowProgress();
    void _u_assureVisibleSelection();
};

#endif // UKUIDIROPERATOR_P_H
