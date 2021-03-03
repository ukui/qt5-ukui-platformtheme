#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QMenu>
#include <QDialog>
#include <QToolBar>
#include <QVariant>
#include <QProxyStyle>
#include <QPushButton>
#include <QPainterPath>
#include <QGraphicsEffect>
#include <qpa/qplatformdialoghelper.h>
#include <peony-qt/directory-view-plugin-iface2.h>
#include <peony-qt/controls/tool-bar/advance-search-bar.h>
#include <peony-qt/controls/tool-bar/view-factory-sort-filter-model.h>
#include <peony-qt/controls/directory-view/directory-view-factory/directory-view-factory-manager.h>

#define QT_CUSTOM_FILE_DIALOG 0


typedef struct _GtkDialog               GtkDialog;
typedef struct _GtkFileFilter           GtkFileFilter;

class QGtk2Dialog;

class HeaderBar;
class FileDialog;
class ViewTypeMenu;
class FileDialogBase;
class QDialogButtonBox;
class FileDialogHelper;
class HeadBarPushButton;
class BorderShadowEffect;

/**
 * FIXME://由于时间不够，先用折中办法，移植 gtk 的filedialog
 */
class FileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
    FileDialogHelper ();
    ~FileDialogHelper () override;

    void exec() override;
    void hide() override;
    bool show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent) override;

    void setFilter() override;
    QUrl directory() const override;
    QList<QUrl> selectedFiles() const override;
    QString selectedNameFilter() const override;
    void selectFile(const QUrl &filename) override;
    bool defaultNameFilterDisables() const override;
    void setDirectory(const QUrl &directory) override;
    void selectNameFilter(const QString &filter) override;

private Q_SLOTS:
    void onAccepted();

private:
    void applyOptions();
    void setNameFilters(const QStringList &filters);
    static void onCurrentFolderChanged(FileDialogHelper* helper);
    static void onSelectionChanged(GtkDialog* dialog, FileDialogHelper* helper);

    QUrl                                mDir;
    QList<QUrl>                         mSelection;
    QScopedPointer<QGtk2Dialog>         mD;
    QHash<QString, GtkFileFilter*>      mFilters;
    QHash<GtkFileFilter*, QString>      mFilterNames;
};
// 以后继续开发自定义文件管理器
// 先用 gtk 实现
#if QT_CUSTOM_FILE_DIALOG
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
    void validBorder();

    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

protected:
    HeaderBar*                  mHeaderBar = nullptr;
    const int                   mWindowMinWidth = 596;

private:
    bool                        mIsDraging = false;
    QPoint                      mOffset;
};


class HeaderBar : public QToolBar
{
    friend class FileDialog;
    friend class HeaderBarContainer;
    Q_OBJECT
private:
    explicit HeaderBar(FileDialog *parent = nullptr);

Q_SIGNALS:
    void updateLocationRequest(const QString &uri, bool addHistory = true, bool force = true);
    void updateSearch(const QString &uri, const QString &key="", bool updateKey=false);
    void viewTypeChangeRequest(const QString &viewId);
    void updateZoomLevelHintRequest(int zoomLevelHint);
    void updateSearchRequest(bool showSearch);

protected:
    void addSpacing(int pixel);
    void mouseMoveEvent(QMouseEvent *e);

private Q_SLOTS:
//    void setLocation(const QString &uri);
//    void cancelEdit();
//    void updateIcons();
//    void updateSearchRecursive(bool recursive);
//    void updateMaximizeState();
//    void startEdit(bool bSearch = false);
//    void finishEdit();
//    void searchButtonClicked();
//    void openDefaultTerminal();
//    void findDefaultTerminal();
//    void tryOpenAgain();
//    void setSearchMode(bool mode);
//    void closeSearch();
//    void initFocus();
//    void updateHeaderState();

private:
    const QString                   mUri;
    FileDialog*                     mWindow;

    Peony::AdvancedLocationBar*     mLocationBar;

    ViewTypeMenu*                   mViewTypeMenu;

    QPushButton*                    mGoBack;
    QPushButton*                    mGoForward;
    QPushButton*                    mSearchButton;

    QWidgetList                     mFocusList;

    const int                       ADDRESS_BAR_MINIMUN_WIDTH = 250;
    const int                       DRAG_AREA_MINIMUN_WIDTH = 32;
    const int                       DRAG_AREA_DEFAULT_WIDTH = 49;
};

class HeadBarPushButton : public QPushButton
{
    friend class HeaderBar;
    friend class FileDialog;
    Q_OBJECT
    explicit HeadBarPushButton(QWidget *parent = nullptr);
};

class ViewTypeMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ViewTypeMenu(QWidget *parent = nullptr);

Q_SIGNALS:
    void switchViewRequest(const QString &viewId, const QIcon &icon, bool resetToZoomLevelHint = false);
    void updateZoomLevelHintRequest(int zoomLevelHint);

public Q_SLOTS:
    void setCurrentView(const QString &viewId, bool blockSignal = false);
    void setCurrentDirectory(const QString &uri);

protected:
    bool isViewIdValid(const QString &viewId);
    void updateMenuActions();

private:
    QString                                 mCurrentUri;
    QString                                 mCurrentViewId;
    QActionGroup*                           mViewActions;
    Peony::ViewFactorySortFilterModel2*     mModel;
};
#endif
#endif // FILEDIALOG_H