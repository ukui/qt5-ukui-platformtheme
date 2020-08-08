#include "ukui-file-widget.h"
#include "ukui-diroperator.h"
#include "ukui-action-collection.h"


#include <PeonyFileItemModel>
#include <PeonyPathCompleter>

#include <QWidget>
#include <QPushButton>
#include <QApplication>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAbstractItemModel>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QTimer>
#include <QCheckBox>
#include <QStyle>
#include <QSplitter>
#include <QListView>



class UKUIFileWidgetPrivate
{
public:
    explicit UKUIFileWidgetPrivate(UKUIFileWidget *widget);
    ~UKUIFileWidgetPrivate();

    QWidget *opsWidget;
    QPushButton *okButton, *cancelButton;
    QToolBar *toolbar;

    QLabel *locationLabel;
    QLabel *filterLabel;
    QComboBox *locationEdit;
    QComboBox *filterWidget;
    QTimer filterDelayTimer;

    QCheckBox *autoSelectExtCheckBox;


    // the last selected url
    QUrl url;
    QString fileClass;

    UKUIDirOperator *ops;

    QSlider *iconSizeSlider;
    QAction *zoomOutAction;
    QAction *zoomInAction;

    QBoxLayout *boxLayout;
    QGridLayout *lafBox;
    QVBoxLayout *vbox;
    QSplitter *placesViewSplitter;
    QListView *placesView;
    Peony::FileItemModel *model;

    UKUIFileWidget * const q;



    void initGUI();
};





UKUIFileWidget::UKUIFileWidget(const QUrl &_startDir, QWidget *parent)
    : QWidget(parent),
      d(new UKUIFileWidgetPrivate(this)),
      operationmode(Saving)
{
    d->okButton = new QPushButton(QIcon::fromTheme("dialog-ok"), QApplication::translate("UKUIStandardGuiItem", "&OK"), this);
    d->okButton->setDefault(true);
    d->cancelButton = new QPushButton(QIcon::fromTheme("dialog-cancel"), QApplication::translate("UKUIStandardGuiItem", "&Cancel"), this);
    // The dialog shows them



    d->model = new Peony::FileItemModel(this);
    d->autoSelectExtCheckBox = new QCheckBox(this);


    d->opsWidget = new QWidget(this);
    QVBoxLayout *opsWidgetLayout = new QVBoxLayout(d->opsWidget);
    opsWidgetLayout->setContentsMargins(0, 0, 0, 0);
    opsWidgetLayout->setSpacing(0);
    d->toolbar = new QToolBar(d->opsWidget);
    d->toolbar->setObjectName(QStringLiteral("UKUIFileWidget::toolbar"));
    d->toolbar->setMovable(false);
    opsWidgetLayout->addWidget(d->toolbar);



    d->ops = new UKUIDirOperator(QUrl(), d->opsWidget);
    d->ops->setObjectName(QStringLiteral("UKUIFileWidget::ops"));
    d->ops->setIsSaving(operationMode());
    opsWidgetLayout->addWidget(d->ops);



    UKUIActionCollection *coll = d->ops->ActionCollection();
    coll->addAssociatedWidget(this);

    d->zoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("file-zoom-out")), tr("Zoom out"), this);
    d->zoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("file-zoom-in")), tr("Zoom in"), this);



    d->iconSizeSlider = new QSlider(this);
    d->iconSizeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    d->iconSizeSlider->setMinimumWidth(40);
    d->iconSizeSlider->setOrientation(Qt::Horizontal);
    d->iconSizeSlider->setMinimum(0);
    d->iconSizeSlider->setMaximum(100);
    d->iconSizeSlider->installEventFilter(this);

    QWidget *midSpacer = new QWidget(this);
    midSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->toolbar->addAction(coll->action(QStringLiteral("back")));
    d->toolbar->addAction(coll->action(QStringLiteral("forward")));
    d->toolbar->addAction(coll->action(QStringLiteral("up")));
    d->toolbar->addAction(coll->action(QStringLiteral("reload")));
    d->toolbar->addSeparator();
    d->toolbar->addAction(coll->action(QStringLiteral("icons view")));
    d->toolbar->addAction(coll->action(QStringLiteral("compact view")));
    d->toolbar->addAction(coll->action(QStringLiteral("details view")));
    d->toolbar->addSeparator();
//    d->toolbar->addAction(coll->action(QStringLiteral("inline preview")));
    d->toolbar->addAction(coll->action(QStringLiteral("sorting menu")));
    d->toolbar->addWidget(midSpacer);
    d->toolbar->addAction(d->zoomOutAction);
    d->toolbar->addWidget(d->iconSizeSlider);
    d->toolbar->addAction(d->zoomInAction);
    d->toolbar->addSeparator();
//    d->toolbar->addAction(coll->action(QStringLiteral("mkdir")));
    d->toolbar->addAction(coll->action(QStringLiteral("option menu")));

    d->toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d->toolbar->setMovable(false);


    // the Location label/edit
    d->locationLabel = new QLabel(tr("&Name:"), this);
    d->locationEdit = new QComboBox(this);
    d->locationEdit->installEventFilter(this);
    // Properly let the dialog be resized (to smaller). Otherwise we could have
    // huge dialogs that can't be resized to smaller (it would be as big as the longest
    // item in this combo box). (ereslibre)
    d->locationEdit->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    d->locationLabel->setBuddy(d->locationEdit);
    Peony::PathCompleter *fileCompletionObj = new Peony::PathCompleter(d->locationEdit);
    d->locationEdit->setCompleter(fileCompletionObj);

    // the Filter label/edit
    d->filterLabel = new QLabel(tr("&File type:"), this);
    d->filterWidget = new QComboBox(this);
    // Properly let the dialog be resized (to smaller). Otherwise we could have
    // huge dialogs that can't be resized to smaller (it would be as big as the longest
    // item in this combo box). (ereslibre)
    d->filterWidget->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    d->filterLabel->setBuddy(d->filterWidget);
    d->filterDelayTimer.setSingleShot(true);
    d->filterDelayTimer.setInterval(300);



    d->initGUI(); // activate GM



    QUrl startDir(_startDir);
    QString filename;
    d->url = getStartUrl(startDir, d->fileClass, filename);
    startDir = d->url;
    d->ops->setUrl(startDir, true);
    d->placesView->setModel(d->model);



    d->locationEdit->setFocus();

}

UKUIFileWidget::~UKUIFileWidget()
{

}


QUrl UKUIFileWidget::getStartUrl(const QUrl &startDir, QString &recentDirClass, QString &fileName)
{
   return startDir;
}

void UKUIFileWidget::setOperationMode(OperationMode mode)
{
    operationmode = mode;
}

UKUIFileWidget::OperationMode UKUIFileWidget::operationMode() const
{
    return operationmode;
}





UKUIFileWidgetPrivate::UKUIFileWidgetPrivate(UKUIFileWidget *widget)
    : q(widget),
      boxLayout(nullptr),
      filterWidget(nullptr),
      toolbar(nullptr),
      ops(nullptr)
{

}

UKUIFileWidgetPrivate::~UKUIFileWidgetPrivate()
{
    delete ops;
}

void UKUIFileWidgetPrivate::initGUI()
{
    delete boxLayout; // deletes all sub layouts

    boxLayout = new QVBoxLayout(q);
    boxLayout->setContentsMargins(0, 0, 0, 0); // no additional margin to the already existing

    placesViewSplitter = new QSplitter(q);
    placesViewSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    placesViewSplitter->setChildrenCollapsible(false);
    boxLayout->addWidget(placesViewSplitter);

    placesViewSplitter->insertWidget(0, opsWidget);

    vbox = new QVBoxLayout();
    vbox->setContentsMargins(0, 0, 0, 0);
    boxLayout->addLayout(vbox);

    lafBox = new QGridLayout();

    lafBox->addWidget(locationLabel, 0, 0, Qt::AlignVCenter | Qt::AlignRight);
    lafBox->addWidget(locationEdit, 0, 1, Qt::AlignVCenter);
    // filedialog has been added,this has been seted hide
//    lafBox->addWidget(okButton, 0, 2, Qt::AlignVCenter);

    lafBox->addWidget(filterLabel, 1, 0, Qt::AlignVCenter | Qt::AlignRight);
    lafBox->addWidget(filterWidget, 1, 1, Qt::AlignVCenter);
    // filedialog has been added,this been seted hide
//    lafBox->addWidget(cancelButton, 1, 2, Qt::AlignVCenter);

    lafBox->setColumnStretch(1, 4);

    vbox->addLayout(lafBox);

    // add the Automatically Select Extension checkbox
    vbox->addWidget(autoSelectExtCheckBox);

    q->setTabOrder(ops, autoSelectExtCheckBox);
    q->setTabOrder(autoSelectExtCheckBox, locationEdit);
    q->setTabOrder(locationEdit, filterWidget);
    q->setTabOrder(filterWidget, okButton);
    q->setTabOrder(okButton, cancelButton);
    q->setTabOrder(cancelButton, ops);
}
