#include "file-dialog.h"

#if QT_CUSTOM_FILE_DIALOG
#include "xatom-helper.h"

#include <QRect>
#include <QRectF>
#include <QDebug>
#include <QDebug>
#include <QWidget>
#include <QWidget>
#include <QWindow>
#include <QPainter>
#include <QX11Info>
#include <QX11Info>
#include <X11/Xlib.h>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QToolButton>
#include <QApplication>
#include <QPainterPath>
#include <KWindowEffects>
#include <QDialogButtonBox>
#endif
#include <private/qguiapplication_p.h>
#include <qpa/qplatformfontdatabase.h>

#undef signals
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <pango/pango.h>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
static GtkFileChooserAction gtkFileChooserAction(const QSharedPointer<QFileDialogOptions> &options);

class QGtk2Dialog : public QWindow
{
    Q_OBJECT
public:
    QGtk2Dialog(GtkWidget *gtkWidget);
    ~QGtk2Dialog();

    GtkDialog *gtkDialog() const;

    void exec();
    bool show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent);
    void hide();

Q_SIGNALS:
    void accept();
    void reject();

protected:
    static void onResponse(QGtk2Dialog *dialog, int response);

private Q_SLOTS:
    void onParentWindowDestroyed();

private:
    GtkWidget *gtkWidget;
};

QGtk2Dialog::QGtk2Dialog(GtkWidget *gtkWidget) : gtkWidget(gtkWidget)
{
    g_signal_connect_swapped(G_OBJECT(gtkWidget), "response", G_CALLBACK(onResponse), this);
    g_signal_connect(G_OBJECT(gtkWidget), "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
}

QGtk2Dialog::~QGtk2Dialog()
{
    gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
    gtk_widget_destroy(gtkWidget);
}

GtkDialog *QGtk2Dialog::gtkDialog() const
{
    return GTK_DIALOG(gtkWidget);
}

void QGtk2Dialog::exec()
{
    if (modality() == Qt::ApplicationModal) {
        // block input to the whole app, including other GTK dialogs
        gtk_dialog_run(gtkDialog());
    } else {
        // block input to the window, allow input to other GTK dialogs
        QEventLoop loop;
        connect(this, SIGNAL(accept()), &loop, SLOT(quit()));
        connect(this, SIGNAL(reject()), &loop, SLOT(quit()));
        loop.exec();
    }
}

bool QGtk2Dialog::show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent)
{
    connect(parent, &QWindow::destroyed, this, &QGtk2Dialog::onParentWindowDestroyed,
            Qt::UniqueConnection);
    setParent(parent);
    setFlags(flags);
    setModality(modality);

    gtk_widget_realize(gtkWidget); // creates X window

    if (parent) {
        XSetTransientForHint(gdk_x11_drawable_get_xdisplay(gtkWidget->window),
                             gdk_x11_drawable_get_xid(gtkWidget->window),
                             parent->winId());
    }

    if (modality != Qt::NonModal) {
        gdk_window_set_modal_hint(gtkWidget->window, true);
        QGuiApplicationPrivate::showModalWindow(this);
    }

    gtk_widget_show(gtkWidget);
    gdk_window_focus(gtkWidget->window, 0);
    return true;
}

void QGtk2Dialog::hide()
{
    QGuiApplicationPrivate::hideModalWindow(this);
    gtk_widget_hide(gtkWidget);
}

void QGtk2Dialog::onResponse(QGtk2Dialog *dialog, int response)
{
    if (response == GTK_RESPONSE_OK) {
        Q_EMIT dialog->accept();
    } else {
        Q_EMIT dialog->reject();
    }
}

void QGtk2Dialog::onParentWindowDestroyed()
{
    // The QGtk2*DialogHelper classes own this object. Make sure the parent doesn't delete it.
    setParent(0);
}

#if QT_CUSTOM_FILE_DIALOG
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
    setContextMenuPolicy(Qt::CustomContextMenu);

    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_TranslucentBackground);
    setProperty("useStyleWindowManager", false);
    setMinimumWidth(mWindowMinWidth);

    setLayout(new QVBoxLayout);

    mHeaderBar = new HeaderBar(this);

    // 添加按钮组 到 widget

    if (QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }

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

void FileDialog::validBorder()
{
    if (this->isMaximized()) {
        setContentsMargins(0, 0, 0, 0);
//        m_effect->setPadding(0);
        setProperty("blurRegion", QVariant());
        KWindowEffects::enableBlurBehind(this->winId(), true);
    } else {
        //setContentsMargins(4, 4, 4, 4);
//        m_effect->setPadding(4);
        QPainterPath path;
        auto rect = this->rect();
        rect.adjust(4, 4, -4, -4);
        path.addRoundedRect(rect, 6, 6);
        //setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
        //use KWindowEffects
        //KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));
        KWindowEffects::enableBlurBehind(this->winId(), true);
    }
}

void FileDialog::paintEvent(QPaintEvent *e)
{
    validBorder();
    QColor color = this->palette().window().color();
    QColor colorBase = this->palette().base().color();

    int R1 = color.red();
    int G1 = color.green();
    int B1 = color.blue();
    qreal a1 = 0.3;

    int R2 = colorBase.red();
    int G2 = colorBase.green();
    int B2 = colorBase.blue();
    qreal a2 = 1;

    qreal a = 1 - (1 - a1)*(1 - a2);

    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

    colorBase.setRed(R);
    colorBase.setGreen(G);
    colorBase.setBlue(B);

//    auto sidebarOpacity = Peony::GlobalSettings::getInstance()->getValue(SIDEBAR_BG_OPACITY).toInt();

//    colorBase.setAlphaF(sidebarOpacity/100.0);

/*    if (qApp->property("blurEnable").isValid()) {
        bool blurEnable = qApp->property("blurEnable").toBool();
        if (!blurEnable) {
            colorBase.setAlphaF(0.9);
        }
    } else*/ {
        colorBase.setAlphaF(0.9);
    }

    QPainterPath sidebarPath;
    sidebarPath.setFillRule(Qt::FillRule::WindingFill);
//    auto adjustedRect = sideBarRect().adjusted(0, 1, 0, 0);
//    sidebarPath.addRoundedRect(adjustedRect, 6, 6);
//    sidebarPath.addRect(adjustedRect.adjusted(0, 0, 0, -6));
//    sidebarPath.addRect(adjustedRect.adjusted(6, 0, 0, 0));
//    m_effect->setTransParentPath(sidebarPath);
//    m_effect->setTransParentAreaBg(colorBase);

    //color.setAlphaF(0.5);
//    m_effect->setWindowBackground(color);
    QPainter p(this);

//    m_effect->drawWindowShadowManually(&p, this->rect(), m_resize_handler->isButtonDown());
    QDialog::paintEvent(e);
}

void FileDialog::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
}

void FileDialog::mouseMoveEvent(QMouseEvent *e)
{
    QDialog::mouseMoveEvent(e);
    if (!mIsDraging) {
        return;
    }

    qreal  dpiRatio = qApp->devicePixelRatio();
    if (QX11Info::isPlatformX11()) {
        Display *display = QX11Info::display();
        Atom netMoveResize = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
        XEvent xEvent;
        const auto pos = QCursor::pos();

        memset(&xEvent, 0, sizeof(XEvent));
        xEvent.xclient.type = ClientMessage;
        xEvent.xclient.message_type = netMoveResize;
        xEvent.xclient.display = display;
        xEvent.xclient.window = this->winId();
        xEvent.xclient.format = 32;
        xEvent.xclient.data.l[0] = pos.x() * dpiRatio;
        xEvent.xclient.data.l[1] = pos.y() * dpiRatio;
        xEvent.xclient.data.l[2] = 8;
        xEvent.xclient.data.l[3] = Button1;
        xEvent.xclient.data.l[4] = 0;

        XUngrabPointer(display, CurrentTime);
        XSendEvent(display, QX11Info::appRootWindow(QX11Info::appScreen()), False, SubstructureNotifyMask | SubstructureRedirectMask, &xEvent);
        //XFlush(display);

        XEvent xevent;
        memset(&xevent, 0, sizeof(XEvent));

        xevent.type = ButtonRelease;
        xevent.xbutton.button = Button1;
        xevent.xbutton.window = this->winId();
        xevent.xbutton.x = e->pos().x() * dpiRatio;
        xevent.xbutton.y = e->pos().y() * dpiRatio;
        xevent.xbutton.x_root = pos.x() * dpiRatio;
        xevent.xbutton.y_root = pos.y() * dpiRatio;
        xevent.xbutton.display = display;

        XSendEvent(display, this->effectiveWinId(), False, ButtonReleaseMask, &xevent);
        XFlush(display);

        if (e->source() == Qt::MouseEventSynthesizedByQt) {
            if (!this->mouseGrabber()) {
                this->grabMouse();
                this->releaseMouse();
            }
        }

        mIsDraging = false;
    } else {
        this->move((QCursor::pos() - mOffset) * dpiRatio);
    }
}

void FileDialog::mousePressEvent(QMouseEvent *e)
{
    QDialog::mousePressEvent(e);
    if (e->button() == Qt::LeftButton && !e->isAccepted()) {
        mIsDraging = true;
        mOffset = mapFromGlobal(QCursor::pos());
    }
}

void FileDialog::mouseReleaseEvent(QMouseEvent *e)
{
    QDialog::mouseReleaseEvent(e);
    mIsDraging = false;
}

HeaderBar::HeaderBar(FileDialog *parent) : QToolBar(parent)
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    setMouseTracking(true);
//    setStyle(HeaderBarStyle::getStyle());
    setFocusPolicy(Qt::TabFocus);

    mWindow = parent;
    setContextMenuPolicy(Qt::CustomContextMenu);
    setStyleSheet(".HeaderBar{"
                  "background-color: transparent;"
                  "border: 0px solid transparent;"
                  "margin: 4px 5px 4px 5px;"
                  "}");

    setMovable(false);

    auto goBack = new HeadBarPushButton(this);
    mGoBack = goBack;
    goBack->setEnabled(false);
    goBack->setToolTip(tr("Go Back"));
    goBack->setFixedSize(QSize(36, 28));
    goBack->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    addWidget(goBack);

    auto goForward = new HeadBarPushButton(this);
    mGoForward = goForward;
    goForward->setEnabled(false);
    goForward->setToolTip(tr("Go Forward"));
    goForward->setFixedSize(QSize(36, 28));
    goForward->setIcon(QIcon::fromTheme("go-next-symbolic"));
    addWidget(goForward);
    connect(goForward, &QPushButton::clicked, mWindow, [=]() {
//        mWindow->getCurrentPage()->goForward();
    });

    addSpacing(9);

    auto locationBar = new Peony::AdvancedLocationBar(this);
    locationBar->setMinimumWidth(ADDRESS_BAR_MINIMUN_WIDTH);
    mLocationBar = locationBar;
    mLocationBar->setFocusPolicy(Qt::FocusPolicy(mLocationBar->focusPolicy() & ~Qt::TabFocus));
    addWidget(locationBar);

    connect(goBack, &QPushButton::clicked, mWindow, [=]() {
//        mWindow->getCurrentPage()->goBack();
        mLocationBar->clearSearchBox();
    });

    connect(mLocationBar, &Peony::AdvancedLocationBar::refreshRequest, [=]() {
//        mWindow->updateTabPageTitle();
    });
    connect(mLocationBar, &Peony::AdvancedLocationBar::updateFileTypeFilter, [=](const int &index) {
//        mWindow->getCurrentPage()->setSortFilter(index);
    });
    connect(mLocationBar, &Peony::AdvancedLocationBar::searchRequest, [=](const QString &path, const QString &key){
//        mWindow->forceStopLoading();
        Q_EMIT this->updateSearch(path, key, true);
    });

    connect(mLocationBar, &Peony::AdvancedLocationBar::updateWindowLocationRequest, this, &HeaderBar::updateLocationRequest);

    addSpacing(9 - 7);

    auto search = new HeadBarPushButton(this);
    mSearchButton = search;
    search->setFixedSize(QSize(40, 40));
    search->setIconSize(QSize(16, 16));
    search->setIcon(QIcon::fromTheme("edit-find-symbolic"));
    search->setToolTip(tr("Search"));
    addWidget(search);
//    connect(search, &QPushButton::clicked, this, &HeaderBar::searchButtonClicked);

    addSpacing(DRAG_AREA_DEFAULT_WIDTH);

    auto a = addAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"));
    auto viewType = qobject_cast<QToolButton *>(widgetForAction(a));
    viewType->setAutoRaise(false);
    viewType->setFixedSize(QSize(57, 40));
    viewType->setIconSize(QSize(16, 16));
    viewType->setPopupMode(QToolButton::InstantPopup);

    mViewTypeMenu = new ViewTypeMenu(viewType);
    a->setMenu(mViewTypeMenu);

    connect(mViewTypeMenu, &ViewTypeMenu::switchViewRequest, this, [=](const QString &id, const QIcon &icon, bool resetToZoomLevel) {
        viewType->setText(id);
        viewType->setIcon(icon);
        this->viewTypeChangeRequest(id);
        if (resetToZoomLevel) {
//            auto viewId = mWindow->getCurrentPage()->getView()->viewId();
//            auto factoryManger = Peony::DirectoryViewFactoryManager2::getInstance();
//            auto factory = factoryManger->getFactory(viewId);
//            int zoomLevelHint = factory->zoom_level_hint();
//            mWindow->getCurrentPage()->setZoomLevelRequest(zoomLevelHint);
        }
    });

    connect(mViewTypeMenu, &ViewTypeMenu::updateZoomLevelHintRequest, this, &HeaderBar::updateZoomLevelHintRequest);

    addSpacing(2);

    a = addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), tr("Sort Type"));
    auto sortType = qobject_cast<QToolButton *>(widgetForAction(a));
    sortType->setAutoRaise(false);
    sortType->setFixedSize(QSize(57, 40));
    sortType->setIconSize(QSize(16, 16));
    sortType->setPopupMode(QToolButton::InstantPopup);

    addSpacing(2);

    a = addAction(QIcon::fromTheme("open-menu-symbolic"), tr("Option"));
    auto popMenu = qobject_cast<QToolButton *>(widgetForAction(a));
    popMenu->setProperty("isOptionButton", true);
    popMenu->setAutoRaise(false);
    popMenu->setFixedSize(QSize(40, 40));
    popMenu->setIconSize(QSize(16, 16));
    popMenu->setPopupMode(QToolButton::InstantPopup);

    search->setFlat(true);
    search->setProperty("isWindowButton", 1);
    search->setProperty("useIconHighlightEffect", 2);
    search->setProperty("isIcon", true);

    goBack->setFlat(true);
    goBack->setProperty("isWindowButton", 1);
    goBack->setProperty("useIconHighlightEffect", 2);
    goBack->setProperty("isIcon", true);
    goForward->setFlat(true);
    goForward->setProperty("isWindowButton", 1);
    goForward->setProperty("useIconHighlightEffect", 2);
    goForward->setProperty("isIcon", true);

    for (auto action : actions()) {
        auto w = widgetForAction(action);

        if (w != search && qobject_cast<QToolButton *>(w)) {
            w->setProperty("isWindowButton", 1);
            if (auto toolButton = qobject_cast<QToolButton *>(w)) {
                toolButton->setAutoRaise(true);
            }
            w->setProperty("useIconHighlightEffect", 2);
        }
    }

    mFocusList<<(goBack);
    mFocusList<<(goForward);
    mFocusList<<(search);
    mFocusList<<(viewType);
    mFocusList<<(sortType);
    mFocusList<<(popMenu);
}

void HeaderBar::addSpacing(int pixel)
{

}

void HeaderBar::mouseMoveEvent(QMouseEvent *e)
{

}

HeadBarPushButton::HeadBarPushButton(QWidget *parent) : QPushButton(parent)
{
    setIconSize(QSize(16, 16));
}

ViewTypeMenu::ViewTypeMenu(QWidget *parent) : QMenu(parent)
{
    mModel = new Peony::ViewFactorySortFilterModel2(this);
    mViewActions = new QActionGroup(this);
    mViewActions->setExclusive(true);

    connect(mViewActions, &QActionGroup::triggered, this, [=](QAction *action) {
        auto viewId = action->data().toString();
        setCurrentView(viewId);
    });

    connect(this, &QMenu::aboutToShow, this, [=]() {
        updateMenuActions();
    });

    setCurrentDirectory("file:///");
}

void ViewTypeMenu::setCurrentView(const QString &viewId, bool blockSignal)
{
    if (viewId == mCurrentViewId) {
        return;
    }

    if (isViewIdValid(viewId)) {
        mCurrentViewId = viewId;
    }

    for (auto action : mViewActions->actions()) {
        if (action->text() == viewId) {
            action->setChecked(true);
        }
    }

    Q_EMIT this->switchViewRequest(viewId, mModel->iconFromViewId(viewId));

    if (!blockSignal) {
        auto factoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
        auto factory = factoryManager->getFactory(viewId);
        int zoomLevelHint = factory->zoom_level_hint();
        Q_EMIT this->updateZoomLevelHintRequest(zoomLevelHint);
    }
}

void ViewTypeMenu::setCurrentDirectory(const QString &uri)
{
    mCurrentUri = uri;
    mModel->setDirectoryUri(uri);
}

bool ViewTypeMenu::isViewIdValid(const QString &viewId)
{

}

void ViewTypeMenu::updateMenuActions()
{

}

#endif

FileDialogHelper::FileDialogHelper()
{
    mD.reset(new QGtk2Dialog(gtk_file_chooser_dialog_new("", 0, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL)));
    connect(mD.data(), SIGNAL(accept()), this, SLOT(onAccepted()));
    connect(mD.data(), SIGNAL(reject()), this, SIGNAL(reject()));

    g_signal_connect(GTK_FILE_CHOOSER(mD->gtkDialog()), "selection-changed", G_CALLBACK(onSelectionChanged), this);
    g_signal_connect_swapped(GTK_FILE_CHOOSER(mD->gtkDialog()), "current-folder-changed", G_CALLBACK(onCurrentFolderChanged), this);
}

FileDialogHelper::~FileDialogHelper()
{

}

void FileDialogHelper::exec()
{
    mD->exec();
}

void FileDialogHelper::hide()
{
    mDir = directory();
    mSelection = selectedFiles();

    mD->hide();
}

bool FileDialogHelper::show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent)
{
    mDir.clear();
    mSelection.clear();

    applyOptions();

    return mD->show(flags, modality, parent);
}

void FileDialogHelper::setFilter()
{
    applyOptions();
}

QUrl FileDialogHelper::directory() const
{
    if (!mDir.isEmpty())
        return mDir;

    QString ret;
    GtkDialog *gtkDialog = mD->gtkDialog();
    gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(gtkDialog));
    if (folder) {
        ret = QString::fromUtf8(folder);
        g_free(folder);
    }

    return QUrl::fromLocalFile(ret);
}

QString FileDialogHelper::selectedNameFilter() const
{
    GtkDialog *gtkDialog = mD->gtkDialog();
    GtkFileFilter *gtkFilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(gtkDialog));
    return mFilterNames.value(gtkFilter);
}

void FileDialogHelper::selectFile(const QUrl &filename)
{
    GtkDialog *gtkDialog = mD->gtkDialog();
    if (options()->acceptMode() == QFileDialogOptions::AcceptSave) {
        QFileInfo fi(filename.toLocalFile());
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gtkDialog), fi.path().toUtf8());
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(gtkDialog), fi.fileName().toUtf8());
    } else {
        gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(gtkDialog), filename.toLocalFile().toUtf8());
    }
}

bool FileDialogHelper::defaultNameFilterDisables() const
{
    return false;
}

void FileDialogHelper::setDirectory(const QUrl &directory)
{
    GtkDialog *gtkDialog = mD->gtkDialog();
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gtkDialog), directory.toLocalFile().toUtf8());
}

void FileDialogHelper::selectNameFilter(const QString &filter)
{
    GtkFileFilter *gtkFilter = mFilters.value(filter);
    if (gtkFilter) {
        GtkDialog *gtkDialog = mD->gtkDialog();
        gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(gtkDialog), gtkFilter);
    }
}

void FileDialogHelper::onAccepted()
{
    Q_EMIT accept();

    QString filter = selectedNameFilter();
    if (filter.isEmpty()) {
        Q_EMIT filterSelected(filter);
    }

    QList<QUrl> files = selectedFiles();
    Q_EMIT filesSelected(files);
    if (files.count() == 1) {
        Q_EMIT fileSelected(files.first());
    }
}

void FileDialogHelper::applyOptions()
{
    GtkDialog *gtkDialog = mD->gtkDialog();
    const QSharedPointer<QFileDialogOptions> &opts = options();

    gtk_window_set_title(GTK_WINDOW(gtkDialog), opts->windowTitle().toUtf8());
    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(gtkDialog), true);

    const GtkFileChooserAction action = gtkFileChooserAction(opts);
    gtk_file_chooser_set_action(GTK_FILE_CHOOSER(gtkDialog), action);

    const bool selectMultiple = opts->fileMode() == QFileDialogOptions::ExistingFiles;
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(gtkDialog), selectMultiple);

    const bool confirmOverwrite = !opts->testOption(QFileDialogOptions::DontConfirmOverwrite);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(gtkDialog), confirmOverwrite);

    const QStringList nameFilters = opts->nameFilters();
    if (!nameFilters.isEmpty()) {
        setNameFilters(nameFilters);
    }

    if (opts->initialDirectory().isLocalFile()) {
        setDirectory(opts->initialDirectory());
    }

    for (auto url = opts->initiallySelectedFiles().constBegin(); url != opts->initiallySelectedFiles().constEnd(); ++url) {
        selectFile(*url);
    }

    const QString initialNameFilter = opts->initiallySelectedNameFilter();
    if (!initialNameFilter.isEmpty()) {
        selectNameFilter(initialNameFilter);
    }

#if GTK_CHECK_VERSION(2, 20, 0)
    GtkWidget *acceptButton = gtk_dialog_get_widget_for_response(gtkDialog, GTK_RESPONSE_OK);
    if (acceptButton) {
        if (opts->isLabelExplicitlySet(QFileDialogOptions::Accept))
            gtk_button_set_label(GTK_BUTTON(acceptButton), opts->labelText(QFileDialogOptions::Accept).toUtf8());
        else if (opts->acceptMode() == QFileDialogOptions::AcceptOpen)
            gtk_button_set_label(GTK_BUTTON(acceptButton), GTK_STOCK_OPEN);
        else
            gtk_button_set_label(GTK_BUTTON(acceptButton), GTK_STOCK_SAVE);
    }

    GtkWidget *rejectButton = gtk_dialog_get_widget_for_response(gtkDialog, GTK_RESPONSE_CANCEL);
    if (rejectButton) {
        if (opts->isLabelExplicitlySet(QFileDialogOptions::Reject)) {
            gtk_button_set_label(GTK_BUTTON(rejectButton), opts->labelText(QFileDialogOptions::Reject).toUtf8());
        } else {
            gtk_button_set_label(GTK_BUTTON(rejectButton), GTK_STOCK_CANCEL);
        }
    }
#endif
}

void FileDialogHelper::setNameFilters(const QStringList &filters)
{
    GtkDialog *gtkDialog = mD->gtkDialog();

    for (auto filter = mFilters.constBegin(); filter != mFilters.constEnd(); ++filter) {
        gtk_file_chooser_remove_filter(GTK_FILE_CHOOSER(gtkDialog), *filter);
    }

    mFilters.clear();
    mFilterNames.clear();

    for (auto filter = filters.constBegin(); filter != filters.constEnd(); ++filter) {
        GtkFileFilter *gtkFilter = gtk_file_filter_new();
        const QString name = filter->left(filter->indexOf(QLatin1Char('(')));
        const QStringList extensions = cleanFilterList(*filter);

        gtk_file_filter_set_name(gtkFilter, name.isEmpty() ? extensions.join(QStringLiteral(", ")).toUtf8() : name.toUtf8());
        for (auto ext = extensions.constBegin(); ext != extensions.constEnd(); ++ext) {
            gtk_file_filter_add_pattern(gtkFilter, ext->toUtf8());
        }

        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtkDialog), gtkFilter);

        mFilters.insert(*filter, gtkFilter);
        mFilterNames.insert(gtkFilter, *filter);
    }
}

void FileDialogHelper::onCurrentFolderChanged(FileDialogHelper *helper)
{
     Q_EMIT helper->directoryEntered(helper->directory());
}

void FileDialogHelper::onSelectionChanged(GtkDialog *dialog, FileDialogHelper *helper)
{
    QString selection;
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
        selection = QString::fromUtf8(filename);
        g_free(filename);
    }
    Q_EMIT helper->currentChanged(QUrl::fromLocalFile(selection));
}

static GtkFileChooserAction gtkFileChooserAction(const QSharedPointer<QFileDialogOptions> &options)
{
    switch (options->fileMode()) {
    case QFileDialogOptions::AnyFile:
    case QFileDialogOptions::ExistingFile:
    case QFileDialogOptions::ExistingFiles:
        if (options->acceptMode() == QFileDialogOptions::AcceptOpen) {
            return GTK_FILE_CHOOSER_ACTION_OPEN;
        } else {
            return GTK_FILE_CHOOSER_ACTION_SAVE;
        }
    case QFileDialogOptions::Directory:
    case QFileDialogOptions::DirectoryOnly:
    default:
        if (options->acceptMode() == QFileDialogOptions::AcceptOpen) {
            return GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        } else {
            return GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER;
        }
    }
}


