#ifndef FILEDIALOGSORTFILTERPROXYMODEL_H
#define FILEDIALOGSORTFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>
#include <QModelIndex>



class FileDialogSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit FileDialogSortFilterProxyModel(QObject *parent = nullptr);

    /**
    * Choose if files and folders are sorted separately (with folders first) or not.
    */
    void setSortFoldersFirst(bool foldersFirst);

protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    bool m_sortFoldersFirst;
};

#endif // FILEDIALOGSORTFILTERPROXYMODEL_H
