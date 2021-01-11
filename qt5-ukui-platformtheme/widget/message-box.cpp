#include "message-box.h"

#include <QPainter>
#include <QVariant>
#include <QVector4D>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QStyleOption>
#include <qpa/qplatformdialoghelper.h>
#include <QtWidgets/qdialogbuttonbox.h>

#include "private/qlabel_p.h"
#include "private/qdialog_p.h"

#include <QIcon>
#include <QLabel>
#include <QScreen>
#include <QCheckBox>
#include <QMetaEnum>
#include <QChildEvent>
#include <QKeySequence>
#include <QAbstractButton>
#include <QAccessibleEvent>
#include <QLineEdit>

enum Button
{
    Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
    Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF, NewButtonMask = 0xFFFFFC00
};

static QMessageBox::StandardButton newButton(int button);
static QMessageDialogOptions::Icon helperIcon(QMessageBox::Icon i);
static QPlatformDialogHelper::StandardButtons helperStandardButtons(MessageBox * q);
static inline bool operator==(const QMessageDialogOptions::CustomButton &a, const QMessageDialogOptions::CustomButton &b);

class MessageBoxPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(MessageBox)

public:
    MessageBoxPrivate ();
    ~MessageBoxPrivate ();

    void init (const QString &title = QString(), const QString &text = QString());

    void setupLayout ();

    void _q_buttonClicked(QAbstractButton*);
    void _q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role);
    void setClickedButton(QAbstractButton *button);

    QAbstractButton* findButton(int button0, int button1, int button2, int flags);
    void addOldButtons(int button0, int button1, int button2);

    QAbstractButton *abstractButtonForId(int id) const;
    int execReturnCode(QAbstractButton *button);

    int dialogCodeForButton(QAbstractButton *button) const;

    void updateSize();
    int layoutMinimumWidth();
    void retranslateStrings();
    void detectEscapeButton();

    static QMessageBox::StandardButton newButton(int button);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, int button0, int button1, int button2);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, const QString &btn0Text, const QString &btn1Text, const QString &btn2Text, int defBtnNum, int espBtnNum);

    static QPixmap standardIcon(QMessageBox::Icon icon, MessageBox *mb);
    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons btn, QMessageBox::StandardButton defBtn);


private:
    void initHelper(QPlatformDialogHelper*) override;
    void helperPrepareShow(QPlatformDialogHelper*) override;
    void helperDone(QDialog::DialogCode, QPlatformDialogHelper*) override;

public:
    QTextEdit*                                          mLabel;                     // qt 显示 label 暂定使用富文本框
    QLabel*                                             mIconLabel;                 // qt 显示图标
    QDialogButtonBox*                                   mButtonBox;                 // qt 按钮框
    QCheckBox*                                          mCheckbox;                  // qt checkbox

    QLayout*                                            mMainLayout;                // 主布局
    QLayout*                                            mButtonLayout;
    QLayout*                                            mContentLayout;

    QByteArray                                          mMemberToDisconnectOnClose;
    QByteArray                                          mSignalToDisconnectOnClose;
    QPointer<QObject>                                   mReceiverToDisconnectOnClose;


    QMessageBox::Icon                                   mIcon;
    QList<QAbstractButton*>                             mCustomButtonList;          // 自定义按钮
    QAbstractButton*                                    mEscapeButton;
    QPushButton*                                        mDefaultButton;
    QAbstractButton*                                    mClickedButton;             // 复选框按钮

    bool                                                mCompatMode;
    bool                                                mAutoAddOkButton;
    QLabel*                                             mInformativeLabel;
    QAbstractButton*                                    mDetectedEscapeButton;
    QSharedPointer<QMessageDialogOptions>               mOptions;

private:
    int                             mRadius = 9;
    int                             mIconSize = 24;

    int                             mMinWidth = 380;
    int                             mMinHeight = 142;
    int                             mMaxWidth = 420;
    int                             mMaxHeight = 562;

    int                             mBtnContent = 32;                   // 底部按钮 与 内容框之间的间隔
    int                             mWidgetSpace = 8;

    int                             mMarginLeft = 32;
    int                             mMarginTop = 32;
    int                             mMarginRight = 32;
    int                             mMarginButton = 24;

    int                             mButtonHeight = 30;
    int                             mButtonWidth = 80;

    int                             mSpace = 8;

};

MessageBox::MessageBox(QWidget *parent) : QDialog(*new MessageBoxPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    Q_D(MessageBox);

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    d->init();

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    setContentsMargins(0, 0, 0, 0);

    setAttribute(Qt::WA_TranslucentBackground);
}

MessageBox::~MessageBox()
{

}

void MessageBox::setCheckBox(QCheckBox *cb)
{
    Q_D(MessageBox);

    if (cb == d->mCheckbox) {
        return;
    }

    if (d->mCheckbox) {
        d->mCheckbox->hide();
        layout()->removeWidget(d->mCheckbox);
        if (d->mCheckbox->parentWidget() == this) {
            d->mCheckbox->setParent(nullptr);
            d->mCheckbox->deleteLater();
        }
    }

    d->mCheckbox = cb;
    if (d->mCheckbox) {
        QSizePolicy sp = d->mCheckbox->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        d->mCheckbox->setSizePolicy(sp);
    }
    d->setupLayout();
}

QCheckBox *MessageBox::checkBox() const
{
    Q_D(const MessageBox);

    return d->mCheckbox;
}

QMessageBox::Icon MessageBox::icon()
{
    Q_D(const MessageBox);

    return d->mIcon;
}

void MessageBox::setIcon(QMessageBox::Icon icon)
{
    Q_D(MessageBox);
    setIconPixmap(MessageBoxPrivate::standardIcon((QMessageBox::Icon)icon, this));
    d->mIcon = icon;
}

QString MessageBox::text()
{
    Q_D(MessageBox);

    return d->mLabel->toPlainText();
}

void MessageBox::setText(const QString& text)
{
    Q_D (MessageBox);

    if (text.at(0) == '<') {
        d->mLabel->setHtml(text);
    } else {
        d->mLabel->setText(text);
    }
}

void MessageBox::addButton(QAbstractButton *button, QMessageBox::ButtonRole role)
{
    Q_D(MessageBox);
    if (!button) {
        return;
    }

    removeButton(button);
    d->mOptions->addButton(button->text(), static_cast<QPlatformDialogHelper::ButtonRole>(role), button);
    d->mButtonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->mCustomButtonList.append(button);
    d->mAutoAddOkButton = false;
}

QPushButton* MessageBox::addButton(const QString &text, QMessageBox::ButtonRole role)
{
    Q_D(MessageBox);
    QPushButton* pushButton = new QPushButton(text);
    addButton(pushButton, role);
    return pushButton;
}

QPushButton *MessageBox::addButton(QMessageBox::StandardButton button)
{
    Q_D(MessageBox);

    if (d->mButtonBox->standardButtons() & button) {
        return nullptr;
    }

    QPushButton *pushButton = d->mButtonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (pushButton) {
        d->mAutoAddOkButton = false;
    }

    pushButton->setIcon(QIcon());

    return pushButton;
}

void MessageBox::removeButton(QAbstractButton *button)
{
    Q_D(MessageBox);

    d->mCustomButtonList.removeAll(button);
    if (d->mEscapeButton == button) {
        d->mEscapeButton = nullptr;
    }

    if (d->mDefaultButton == button) {
        d->mDefaultButton = nullptr;
    }

    d->mButtonBox->removeButton(button);
}

QAbstractButton *MessageBox::button(QMessageBox::StandardButton which) const
{
    Q_D(const MessageBox);

    return d->mButtonBox->button(QDialogButtonBox::StandardButton(which));
}

QList<QAbstractButton*> MessageBox::buttons() const
{
    Q_D(const MessageBox);

    return d->mButtonBox->buttons();
}

QMessageBox::ButtonRole MessageBox::buttonRole(QAbstractButton *button) const
{
    Q_D(const MessageBox);

    return QMessageBox::ButtonRole(d->mButtonBox->buttonRole(button));
}

void MessageBox::setStandardButtons(QMessageBox::StandardButtons buttons)
{
    Q_D(MessageBox);

    d->mButtonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    QList<QAbstractButton *> buttonList = d->mButtonBox->buttons();
    if (!buttonList.contains(d->mEscapeButton)) {
        d->mEscapeButton = nullptr;
    }

    if (!buttonList.contains(d->mDefaultButton)) {
        d->mDefaultButton = nullptr;
    }

    d->mAutoAddOkButton = false;
}

QMessageBox::StandardButtons MessageBox::standardButtons() const
{
    Q_D(const MessageBox);

    return QMessageBox::StandardButtons(int(d->mButtonBox->standardButtons()));
}

QMessageBox::StandardButton MessageBox::standardButton(QAbstractButton *button) const
{
    Q_D(const MessageBox);
    return (QMessageBox::StandardButton)d->mButtonBox->standardButton(button);
}

QPushButton *MessageBox::defaultButton() const
{
    Q_D(const MessageBox);

    return d->mDefaultButton;
}

void MessageBox::setDefaultButton(QPushButton *button)
{
    Q_D(MessageBox);

    if (!d->mButtonBox->buttons().contains(button)) {
        return;
    }

    d->mDefaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

void MessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    Q_D(MessageBox);
    setDefaultButton(d->mButtonBox->button(QDialogButtonBox::StandardButton(button)));
}

QAbstractButton* MessageBox::escapeButton() const
{
    Q_D(const MessageBox);
    return d->mEscapeButton;
}

void MessageBox::setEscapeButton(QAbstractButton *button)
{
    Q_D(MessageBox);

    if (d->mButtonBox->buttons().contains(button)) {
        d->mEscapeButton = button;
    }
}

void MessageBox::setEscapeButton(QMessageBox::StandardButton button)
{
    Q_D(MessageBox);

    setEscapeButton(d->mButtonBox->button(QDialogButtonBox::StandardButton(button)));
}

QAbstractButton* MessageBox::clickedButton() const
{
    Q_D(const MessageBox);

    return d->mClickedButton;
}

QString MessageBox::buttonText(int button) const
{
    Q_D(const MessageBox);

    if (QAbstractButton *abstractButton = d->abstractButtonForId(button)) {
        return abstractButton->text();
    } else if (d->mButtonBox->buttons().isEmpty() && (button == QMessageBox::Ok || button == Old_Ok)) {
        return QDialogButtonBox::tr("OK");
    }
    return QString();
}

void MessageBox::setButtonText(int button, const QString &text)
{
    Q_D(MessageBox);
    if (QAbstractButton *abstractButton = d->abstractButtonForId(button)) {
        abstractButton->setText(text);
    } else if (d->mButtonBox->buttons().isEmpty() && (button == QMessageBox::Ok || button == Old_Ok)) {
        addButton(QMessageBox::Ok)->setText(text);
    }
}

QString MessageBox::informativeText() const
{
    Q_D(const MessageBox);

    return d->mInformativeLabel ? d->mInformativeLabel->text() : QString();
}

void MessageBox::setInformativeText(const QString &text)
{
    Q_D(MessageBox);
    if (text.isEmpty()) {
        if (d->mInformativeLabel) {
            d->mInformativeLabel->hide();
            d->mInformativeLabel->deleteLater();
        }
        d->mInformativeLabel = nullptr;
    } else {
        if (!d->mInformativeLabel) {
            QLabel *label = new QLabel;
            label->setObjectName(QLatin1String("ukui_msgbox_informativelabel"));
            label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this)));
            label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            label->setOpenExternalLinks(true);
            label->setWordWrap(true);
    #ifdef Q_OS_MAC
            label->setFont(qt_app_fonts_hash()->value("QTipLabel"));
    #endif
            label->setWordWrap(true);
            d->mInformativeLabel = label;
        }
        d->mInformativeLabel->setText(text);
    }
    d->setupLayout();
}

QPixmap MessageBox::iconPixmap() const
{
    Q_D(const MessageBox);

    return *(d->mIconLabel->pixmap());
}

void MessageBox::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(MessageBox);
    d->mIconLabel->setPixmap(pixmap.scaled(d->mIconSize, d->mIconSize));
    d->mIcon = QMessageBox::NoIcon;
    d->setupLayout();
}

void MessageBox::setWindowTitle(const QString &title)
{
#ifndef Q_OS_MAC
    QDialog::setWindowTitle(title);
#else
    Q_UNUSED(title);
#endif
}

void MessageBox::setWindowModality(Qt::WindowModality windowModality)
{
    QDialog::setWindowModality(windowModality);

    if (parentWidget() && windowModality == Qt::WindowModal) {
        setParent(parentWidget(), Qt::Sheet);
    } else {
        setParent(parentWidget(), Qt::Dialog);
    }

    setDefaultButton(d_func()->mDefaultButton);
}

QPixmap MessageBox::standardIcon(QMessageBox::Icon icon)
{
    return MessageBoxPrivate::standardIcon(icon, nullptr);
}

bool MessageBox::event(QEvent *e)
{
    bool result = QDialog::event(e);
    switch (e->type()) {
        case QEvent::LayoutRequest:
            d_func()->updateSize();
            break;
        case QEvent::LanguageChange:
            d_func()->retranslateStrings();
            break;
        default:
            break;
    }
    return result;
}

void MessageBox::changeEvent(QEvent *event)
{
    Q_D(MessageBox);
    switch (event->type()) {
    case QEvent::StyleChange:
    {
        if (d->mIcon != QMessageBox::NoIcon)
            setIcon(d->mIcon);
        Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, this));
        d->mLabel->setTextInteractionFlags(flags);
        d->mButtonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, this));
        if (d->mInformativeLabel)
            d->mInformativeLabel->setTextInteractionFlags(flags);
        Q_FALLTHROUGH();
    }
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
#ifdef Q_OS_MAC
    {
        QFont f = font();
        f.setBold(true);
        d->label->setFont(f);
    }
#endif
        Q_FALLTHROUGH();
    default:
        break;
    }
    QDialog::changeEvent(event);
}

void MessageBox::showEvent(QShowEvent *event)
{
    Q_D(MessageBox);
    if (d->mAutoAddOkButton) {
        addButton(QMessageBox::Ok);
    }

    d->detectEscapeButton();
    d->updateSize();

#ifndef QT_NO_ACCESSIBILITY
    QAccessibleEvent e(this, QAccessible::Alert);
    QAccessible::updateAccessibility(&e);
#endif
    QDialog::showEvent(event);
}

void MessageBox::closeEvent(QCloseEvent *event)
{
    Q_D(MessageBox);
    if (!d->mDetectedEscapeButton) {
        event->ignore();
        return;
    }

    QDialog::closeEvent(event);
    d->mClickedButton = d->mDetectedEscapeButton;
    setResult(d->execReturnCode(d->mDetectedEscapeButton));
}

void MessageBox::keyPressEvent(QKeyEvent* e)
{
#if QT_CONFIG(shortcut)
    Q_D(MessageBox);
    if (e->matches(QKeySequence::Cancel)) {
        if (d->mDetectedEscapeButton) {
#ifdef Q_OS_MAC
            d->detectedEscapeButton->animateClick();
#else
            d->mDetectedEscapeButton->click();
#endif
        }
        return;
    }
#endif // QT_CONFIG(shortcut)

#ifndef QT_NO_SHORTCUT
    if (!(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier | Qt::MetaModifier))) {
        int key = e->key() & ~Qt::MODIFIER_MASK;
        if (key) {
            const QList<QAbstractButton *> buttons = d->mButtonBox->buttons();
            for (auto pb = buttons.constBegin(); pb != buttons.constEnd(); ++pb) {
                QKeySequence shortcut = (*pb)->shortcut();
                if (!shortcut.isEmpty() && key == shortcut[0]) {
                    (*pb)->animateClick();
                    return;
                }
            }
        }
    }

#endif
    QDialog::keyPressEvent(e);
}

void MessageBox::paintEvent(QPaintEvent *event)
{
    Q_D(MessageBox);

    QPainter painter (this);
    QPalette palette;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(palette.brush(QPalette::Base));
    painter.drawRoundedRect(rect(), d->mRadius, d->mRadius);

    painter.restore();

    QWidget::paintEvent(event);
}

void MessageBox::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}

void MessageBox::initHelper(QPlatformMessageDialogHelper* h)
{
    Q_D(MessageBox);

    d->initHelper(h);
}

class MessageBoxOptionsPrivate : public QSharedData
{
public:
    MessageBoxOptionsPrivate() :
        icon(QMessageDialogOptions::NoIcon),
        buttons(QPlatformDialogHelper::Ok),
        nextCustomButtonId(QPlatformDialogHelper::LastButton + 1)
    {
    }

    QString windowTitle;
    QMessageDialogOptions::Icon icon;
    QString text;
    QString informativeText;
    QString detailedText;
    QPlatformDialogHelper::StandardButtons buttons;
    QList<QMessageDialogOptions::CustomButton> customButtons;
    int nextCustomButtonId;
};

MessageBoxHelper::MessageBoxHelper() : QPlatformMessageDialogHelper(), mMessageBox(new MessageBox)
{

}

MessageBoxHelper::~MessageBoxHelper()
{

}

void MessageBoxHelper::exec()
{
    int ret = mMessageBox->exec();
    int role = mMessageBox->buttonRole(mMessageBox->clickedButton());

    mMessageBox->done(ret);

    Q_EMIT clicked((QPlatformDialogHelper::StandardButton)ret, QPlatformDialogHelper::ButtonRole(role));
}

void MessageBoxHelper::hide()
{
    mMessageBox->hide();
}

bool MessageBoxHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    initDialog ();

    mMessageBox->show();
    mMessageBox->d_func()->updateSize();

    Q_UNUSED(parent);
    Q_UNUSED(windowFlags);
    Q_UNUSED(windowModality);

    return true;
}

void MessageBoxHelper::initDialog()
{
    mMessageBox->setIcon((QMessageBox::Icon)options()->icon());
    mMessageBox->setText(options()->text());

    QPlatformDialogHelper::StandardButtons btns = options()->standardButtons();


    uint mask = QMessageBox::FirstButton;

    while (mask <= QMessageBox::LastButton) {
        uint sb = btns & mask;
        mask <<= 1;
        if (!sb) {
            continue;
        }

        mMessageBox->addButton((QMessageBox::StandardButton)sb);
    }
}


MessageBoxPrivate::MessageBoxPrivate() : mCheckbox(nullptr), mEscapeButton(nullptr), mDefaultButton(nullptr), mClickedButton(nullptr), mCompatMode(false), mAutoAddOkButton(true),
    mDetectedEscapeButton(nullptr), mInformativeLabel(nullptr), mOptions(QMessageDialogOptions::create())
{
}

MessageBoxPrivate::~MessageBoxPrivate()
{
    if (nullptr != mLabel) {
        delete mLabel;
    }

    if (nullptr != mIconLabel) {
        delete mIconLabel;
    }

    if (nullptr != mButtonBox) {
        delete mButtonBox;
    }

    if (nullptr != mCheckbox) {
        delete mCheckbox;
    }
}

void MessageBoxPrivate::init(const QString &title, const QString &text)
{
    Q_Q(MessageBox);

    mLabel = new QTextEdit;
    mLabel->setObjectName(QLatin1String("ukui_msgbox_label"));
//    mLabel->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, nullptr, q)));
    mLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//    mLabel->setOpenExternalLinks(true);
//    mLabel->setWordWrap(true);
    mLabel->setContentsMargins(8, 0, 0, 0);
    mLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mLabel->setBackgroundRole(QPalette::Base);
    mLabel->setFrameShape(QTextEdit::NoFrame);

    mIconLabel = new QLabel(q);
    mIconLabel->setObjectName(QLatin1String("ukui_msgbox_icon_label"));
    mIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mIconLabel->setFixedSize(mIconSize, mIconSize);
    mIconLabel->setContentsMargins(0, 0, 0, 0);

    mButtonBox = new QDialogButtonBox;
    mButtonBox->setObjectName(QLatin1String("ukui_msgbox_buttonbox"));
    mButtonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, q));
    QObject::connect(mButtonBox, SIGNAL(clicked(QAbstractButton*)), q, SLOT(_q_buttonClicked(QAbstractButton*)));

    setupLayout();

    if (!text.isEmpty()) {
        q->setText (text);
    }

    q->setModal(true);
    mIcon = QMessageBox::NoIcon;

    Q_UNUSED(title);
}

void MessageBoxPrivate::setupLayout()
{
    Q_Q(MessageBox);

    if (q->layout()) {
        delete q->layout();
    }

    mMainLayout = new QGridLayout;
    mContentLayout = new QHBoxLayout;
    mButtonLayout = new QHBoxLayout;

    mMainLayout->setContentsMargins(0, 0, 0, 0);
    mContentLayout->setContentsMargins(0, 0, 0, 0);
    mButtonLayout->setContentsMargins(0, 0, 0, 0);

    mContentLayout->addWidget(mIconLabel);
    mContentLayout->addWidget(mLabel);

    mButtonLayout->addWidget(mButtonBox);

    mMainLayout->addItem(mContentLayout);
    mMainLayout->addItem(mButtonLayout);

    mContentLayout->setAlignment(mIconLabel, Qt::AlignLeft | Qt::AlignTop);
    mContentLayout->setAlignment(mLabel, Qt::AlignLeft | Qt::AlignTop);

    q->setContentsMargins(mMarginLeft, mMarginLeft, mMarginLeft, mMarginLeft);
    q->setLayout(mMainLayout);
}

void MessageBoxPrivate::_q_buttonClicked(QAbstractButton* button)
{
    Q_Q(MessageBox);

    setClickedButton(button);

    if (mReceiverToDisconnectOnClose) {
        QObject::disconnect(q, mSignalToDisconnectOnClose, mReceiverToDisconnectOnClose, mMemberToDisconnectOnClose);
        mReceiverToDisconnectOnClose = nullptr;
    }

    mSignalToDisconnectOnClose.clear();
    mMemberToDisconnectOnClose.clear();
}

void MessageBoxPrivate::_q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role)
{
    Q_Q(MessageBox);
    if (button > QPlatformDialogHelper::LastButton) {
        mClickedButton = static_cast<QAbstractButton *>(mOptions->customButton(button)->button);
        Q_ASSERT(mClickedButton);
        mClickedButton->click();
        q->done(role);
    } else {
        q->done(button);
    }
}

// after click button
void MessageBoxPrivate::setClickedButton(QAbstractButton *button)
{
    Q_Q(MessageBox);

    mClickedButton = button;
    Q_EMIT q->buttonClicked(mClickedButton);

    mClickedButton = button;

    auto resultCode = execReturnCode(button);
    hide(resultCode);
    finalize(resultCode, dialogCodeForButton(button));
}

QAbstractButton* MessageBoxPrivate::findButton(int button0, int button1, int button2, int flags)
{
    Q_Q(MessageBox);
    int button = 0;

    if (button0 & flags) {
        button = button0;
    } else if (button1 & flags) {
        button = button1;
    } else if (button2 & flags) {
        button = button2;
    }
    return q->button(newButton(button));
}

static bool detectedCompat(int button0, int button1, int button2)
{
    if (button0 != 0 && !(button0 & NewButtonMask))
        return true;
    if (button1 != 0 && !(button1 & NewButtonMask))
        return true;
    if (button2 != 0 && !(button2 & NewButtonMask))
        return true;
    return false;
}

void MessageBoxPrivate::addOldButtons(int button0, int button1, int button2)
{
    Q_Q(MessageBox);
    q->addButton(newButton(button0));
    q->addButton(newButton(button1));
    q->addButton(newButton(button2));
    q->setDefaultButton(static_cast<QPushButton *>(findButton(button0, button1, button2, QMessageBox::Default)));
    q->setEscapeButton(findButton(button0, button1, button2, QMessageBox::Escape));
    mCompatMode = detectedCompat(button0, button1, button2);
}

QAbstractButton* MessageBoxPrivate::abstractButtonForId(int id) const
{
    Q_Q(const MessageBox);
    QAbstractButton *result = mCustomButtonList.value(id);
    if (result) {
        return result;
    }

    if (id & QMessageBox::FlagMask) {
        return nullptr;
    }

    return q->button(newButton(id));
}

int MessageBoxPrivate::execReturnCode(QAbstractButton *button)
{
    int ret = mButtonBox->standardButton(button);
    if (ret == QMessageBox::NoButton) {
        ret = mCustomButtonList.indexOf(button);
    } else if (mCompatMode) {
        ret = -1;
    }

    return ret;
}

int MessageBoxPrivate::dialogCodeForButton(QAbstractButton *button) const
{
    Q_Q(const MessageBox);

    switch (q->buttonRole(button)) {
    case QMessageBox::AcceptRole:
    case QMessageBox::YesRole:
        return QDialog::Accepted;
    case QMessageBox::RejectRole:
    case QMessageBox::NoRole:
        return QDialog::Rejected;
    default:
        return -1;
    }
}

void MessageBoxPrivate::detectEscapeButton()
{
    if (mEscapeButton) {
        mDetectedEscapeButton = mEscapeButton;
        return;
    }

    mDetectedEscapeButton = mButtonBox->button(QDialogButtonBox::Cancel);
    if (mDetectedEscapeButton) {
        return;
    }

    const QList<QAbstractButton *> buttons = mButtonBox->buttons();
    if (buttons.count() == 1) {
        mDetectedEscapeButton = buttons.first();
        return;
    }

    for (auto *button : buttons) {
        if (mButtonBox->buttonRole(button) == QDialogButtonBox::RejectRole) {
            if (mDetectedEscapeButton) { // already detected!
                mDetectedEscapeButton = nullptr;
                break;
            }
            mDetectedEscapeButton = button;
        }
    }

    if (mDetectedEscapeButton) {
        return;
    }

    for (auto *button : buttons) {
        if (mButtonBox->buttonRole(button) == QDialogButtonBox::NoRole) {
            if (mDetectedEscapeButton) { // already detected!
                mDetectedEscapeButton = nullptr;
                break;
            }
            mDetectedEscapeButton = button;
        }
    }
}

#include <syslog.h>


QString SpliteText(const QFontMetrics& font, const QString& text, int nLabelSize)
{
    int nTextSize = font.width(text);
    if (nTextSize >= nLabelSize) {
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); ++i) {
            nOffset += font.width(text.at(i));
            if(nOffset > nLabelSize) {
                nPos = i;
                break;
            }
        }

        nPos = (nPos - 1 < 0) ? 0 : nPos - 1;

        int enter = 0;
        while (text.at(nPos - enter) == '\n') {++ enter;}
        QString qstrLeftData = text.left(nPos);

        enter = 0;
        while (text.at(nPos + enter) == '\n') {++ enter;}
        QString qstrMidData = text.mid(nPos);

        return qstrLeftData + "\n" + SpliteText(font, qstrMidData, nLabelSize);
    }

    return text;
}


// FIXME://
void MessageBoxPrivate::updateSize()
{
    Q_Q(MessageBox);

    if (!q->isVisible()) {
        return;
    }

    // check icon
    /*if (QMessageBox::NoIcon == mIcon) {
        mIconLabel->setFixedSize(QSize(0, 0));
    } else*/ {
        mIconLabel->setFixedSize(QSize(mIconSize, mIconSize));
    }

    // get button numbers
    int                 buttonNum;
    int                 checkButton = mCheckbox == nullptr ? 0 : 1;
    int                 textTmpWidth = 0;
    int                 textTmpHeight = 0;

    int                 fixWidth = 0;
    int                 fixHeight = 0;

    int                 allFontWidth = 0;
    int                 allFontHeight = 0;

    buttonNum = mCustomButtonList.size();
    mLabel->setContentsMargins(0, 0, 0, 0);

    allFontWidth = mLabel->fontMetrics().averageCharWidth() * mLabel->toPlainText().size();

    /*if (QMessageBox::NoIcon == mIcon) {
        textTmpWidth = mMarginLeft + mMarginRight + mWidgetSpace;
        textTmpHeight = mMarginTop + mMarginButton + mButtonHeight + mBtnContent;
    } else*/ {
        textTmpWidth = mMarginLeft + mMarginRight + mWidgetSpace + mIconSize;
        textTmpHeight = mMarginTop + mMarginButton + mButtonHeight + mBtnContent;
    }

    if (allFontWidth + textTmpWidth >= mMaxWidth) {
        fixWidth = mMaxWidth;
    } else if (allFontWidth + textTmpWidth <= mMinWidth) {
        fixWidth = mMinWidth;
    } else {
        fixWidth = allFontWidth + textTmpWidth;
    }

    allFontHeight = allFontWidth / (fixWidth - textTmpHeight) * mLabel->fontMetrics().height();

    if (allFontHeight + textTmpHeight >= mMaxHeight) {
        fixHeight = mMaxHeight;
    } else if (allFontHeight + textTmpHeight <= mMinHeight) {
        fixHeight = mMinHeight;
    } else {
        fixHeight = allFontHeight + textTmpHeight;
    }

    mLabel->setFixedWidth(qAbs(fixWidth - textTmpWidth));
    mLabel->setFixedHeight(qAbs(fixHeight - textTmpHeight));

//    QFontMetrics font(mLabel->font());
//    int nTextSize = font.width(mLabel->toPlainText());
//    int nLabelSize = mLabel->width();
//    if(nTextSize > nLabelSize){
//        QString qstrData = SpliteText(font, mLabel->toPlainText(), nLabelSize);
//        mLabel->setText(qstrData);
//        mLabel->setStyleSheet("background-color:red;");
//    }



//    syslog (LOG_ERR, "???%s", text.toUtf8().constData());

//    if (!text.isEmpty()) {
//        for (int i = 0; i < text.size(); ++i) {
//            if ((fm.horizontalAdvance(tmp) >= fixWidth - textTmpWidth - 10)
//                    || (text.at(i) == enter || text.at(i + 1) == enter)) {

//                formatText += tmp;
//                if (text.at(i) != enter && text.at(i + 1) != enter) {
//                    formatText += enter;
//                }
//                tmp = "";
//            }
//            tmp += text.at(i);
//        }
//        formatText += tmp;
//    }

//    mLabel->setText(formatText);


    // FIXME://
    q->setFixedSize(fixWidth, fixHeight);

    QCoreApplication::removePostedEvents(q, QEvent::LayoutRequest);
}

int MessageBoxPrivate::layoutMinimumWidth()
{
    layout->activate();

    return layout->totalMinimumSize().width();
}

// ok
void MessageBoxPrivate::retranslateStrings()
{

}

QMessageBox::StandardButton MessageBoxPrivate::newButton(int button)
{
    if (button == QMessageBox::NoButton) {
        return QMessageBox::StandardButton(button & QMessageBox::ButtonMask);
    }

    return QMessageBox::NoButton;
}

int MessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, int button0, int button1, int button2)
{
    MessageBox messageBox;
    messageBox.setIcon(icon);
    messageBox.setText(text);
    messageBox.setWindowTitle(title);
    messageBox.d_func()->addOldButtons(button0, button1, button2);

    Q_UNUSED(parent);

    return messageBox.exec();
}

int MessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, const QString &button0Text, const QString &button1Text, const QString &button2Text, int defaultButtonNumber, int escapeButtonNumber)
{
    MessageBox messageBox;
    messageBox.setIcon(icon);
    messageBox.setText(text);
    messageBox.setWindowTitle(title);

    QString myButton0Text = button0Text;
    if (myButton0Text.isEmpty()) {
        myButton0Text = QDialogButtonBox::tr("OK");
    }

    messageBox.addButton(myButton0Text, QMessageBox::ActionRole);

    if (!button1Text.isEmpty()) {
        messageBox.addButton(button1Text, QMessageBox::ActionRole);
    }
    if (!button2Text.isEmpty()) {
        messageBox.addButton(button2Text, QMessageBox::ActionRole);
    }

    const QList<QAbstractButton *> &buttonList = messageBox.d_func()->mCustomButtonList;
    messageBox.setDefaultButton(static_cast<QPushButton *>(buttonList.value(defaultButtonNumber)));
    messageBox.setEscapeButton(buttonList.value(escapeButtonNumber));

    Q_UNUSED(parent);

    return messageBox.exec();
}

QMessageBox::StandardButton MessageBoxPrivate::showNewMessageBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    if (defaultButton && !(buttons & defaultButton)) {
        return (QMessageBox::StandardButton) MessageBoxPrivate::showOldMessageBox(parent, icon, title, text, int(buttons), int(defaultButton), 0);
    }

    MessageBox messageBox;
    messageBox.setIcon(icon);
    messageBox.setText(text);
    messageBox.setWindowTitle(title);
    QDialogButtonBox *buttonBox = messageBox.findChild<QDialogButtonBox*>();
    Q_ASSERT(buttonBox != nullptr);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb) {
            continue;
        }
        QPushButton *button = messageBox.addButton((QMessageBox::StandardButton)sb);
        if (messageBox.defaultButton()) {
            continue;
        }
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton))) {
            messageBox.setDefaultButton(button);
        }
    }

    if (messageBox.exec() == -1) {
        return QMessageBox::Cancel;
    }

    return messageBox.standardButton(messageBox.clickedButton());
}

QPixmap MessageBoxPrivate::standardIcon(QMessageBox::Icon icon, MessageBox *mb)
{
    QStyle *style = mb ? mb->style() : QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, nullptr, mb);
    QIcon tmpIcon;
    switch (icon) {
    case QMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, nullptr, mb);
        break;
    case QMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, nullptr, mb);
        break;
    case QMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, nullptr, mb);
        break;
    case QMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, nullptr, mb);
    default:
        break;
    }
    if (!tmpIcon.isNull()) {
//        qreal dpr = mb ? mb->devicePixelRatio() : qApp->devicePixelRatio();
        return tmpIcon.pixmap (QSize(iconSize, iconSize));
    }

    return QPixmap();
}

void MessageBoxPrivate::initHelper(QPlatformDialogHelper* h)
{
    Q_Q(MessageBox);
    QObject::connect(h, SIGNAL(clicked(QPlatformDialogHelper::StandardButton,QPlatformDialogHelper::ButtonRole)), q, SLOT(_q_clicked(QPlatformDialogHelper::StandardButton,QPlatformDialogHelper::ButtonRole)));
    static_cast<QPlatformMessageDialogHelper*>(h)->setOptions(mOptions);
}

void MessageBoxPrivate::helperPrepareShow(QPlatformDialogHelper*)
{
    Q_Q(MessageBox);
    mOptions->setWindowTitle(q->windowTitle());
    mOptions->setText(q->text());
    mOptions->setInformativeText(q->informativeText());

    mOptions->setIcon(helperIcon(q->icon()));
    mOptions->setStandardButtons(helperStandardButtons(q));
}

void MessageBoxPrivate::helperDone(QDialog::DialogCode code, QPlatformDialogHelper*)
{
    Q_Q(MessageBox);

    QAbstractButton *button = q->button(QMessageBox::StandardButton(code));
    if (button) {
        mClickedButton = button;
    }
}

static QPlatformDialogHelper::StandardButtons helperStandardButtons(MessageBox * q)
{
    QPlatformDialogHelper::StandardButtons buttons(int(q->standardButtons()));

    return buttons;
}

static QMessageDialogOptions::Icon helperIcon(QMessageBox::Icon i)
{
    switch (i) {
    case QMessageBox::NoIcon:
        return QMessageDialogOptions::NoIcon;
    case QMessageBox::Information:
        return QMessageDialogOptions::Information;
    case QMessageBox::Warning:
        return QMessageDialogOptions::Warning;
    case QMessageBox::Critical:
        return QMessageDialogOptions::Critical;
    case QMessageBox::Question:
        return QMessageDialogOptions::Question;
    }
    return QMessageDialogOptions::NoIcon;
}

static QMessageBox::StandardButton newButton(int button)
{
    // this is needed for source compatibility with Qt 4.0 and 4.1
    if (button == QMessageBox::NoButton || (button & NewButtonMask))
        return QMessageBox::StandardButton(button & QMessageBox::ButtonMask);

#if QT_VERSION < 0x050000
    // this is needed for binary compatibility with Qt 4.0 and 4.1
    switch (button & Old_ButtonMask) {
    case Old_Ok:
        return QMessageBox::Ok;
    case Old_Cancel:
        return QMessageBox::Cancel;
    case Old_Yes:
        return QMessageBox::Yes;
    case Old_No:
        return QMessageBox::No;
    case Old_Abort:
        return QMessageBox::Abort;
    case Old_Retry:
        return QMessageBox::Retry;
    case Old_Ignore:
        return QMessageBox::Ignore;
    case Old_YesAll:
        return QMessageBox::YesToAll;
    case Old_NoAll:
        return QMessageBox::NoToAll;
    default:
        return QMessageBox::NoButton;
    }
#else
    return QMessageBox::NoButton;
#endif
}


#include "moc_message-box.cpp"
//#include "messagebox.moc"
