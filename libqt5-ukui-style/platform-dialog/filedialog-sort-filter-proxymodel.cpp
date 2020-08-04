#include "filedialog-sort-filter-proxymodel.h"

FileDialogSortFilterProxyModel::FileDialogSortFilterProxyModel(QObject * parent)
    : QSortFilterProxyModel(parent)
{

}



void FileDialogSortFilterProxyModel::setSortFoldersFirst(bool foldersFirst)
{
    m_sortFoldersFirst = foldersFirst;
}



bool FileDialogSortFilterProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    return QSortFilterProxyModel::filterAcceptsColumn(source_column, source_parent);
}

bool FileDialogSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
