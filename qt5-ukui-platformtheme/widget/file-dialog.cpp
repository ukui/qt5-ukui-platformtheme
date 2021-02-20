#include "file-dialog.h"
#include "xatom-helper.h"

#include <QDebug>
#include <QDebug>
#include <QWidget>
#include <QWidget>
#include <QWindow>
#include <QPainter>
#include <QX11Info>
#include <X11/Xlib.h>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPainterPath>
#include <QRectF>
#include <QRect>
#include <QX11Info>
#include <KWindowEffects>
#include <QMouseEvent>
#include <QApplication>
#include <QToolButton>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

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

FileDialogHelper::FileDialogHelper() : QPlatformFileDialogHelper(), mDialog(new FileDialog)
{
    connect(mDialog, &FileDialogBase::closed, this, &FileDialogHelper::saveSize);
    connect(mDialog, &QDialog::finished, this, &FileDialogHelper::saveSize);
    connect(mDialog, &FileDialogBase::currentChanged, this, &QPlatformFileDialogHelper::currentChanged);
    connect(mDialog, &FileDialogBase::directoryEntered, this, &QPlatformFileDialogHelper::directoryEntered);
    connect(mDialog, &FileDialogBase::fileSelected, this, &QPlatformFileDialogHelper::fileSelected);
    connect(mDialog, &FileDialogBase::filesSelected, this, &QPlatformFileDialogHelper::filesSelected);
    connect(mDialog, &FileDialogBase::filterSelected, this, &QPlatformFileDialogHelper::filterSelected);
    connect(mDialog, &QDialog::accepted, this, &QPlatformDialogHelper::accept);
    connect(mDialog, &QDialog::rejected, this, &QPlatformDialogHelper::reject);
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
