/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "message-box.h"
#include "xatom-helper.h"

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
#include <QtMath>
#include <QLabel>
#include <QScreen>
#include <QCheckBox>
#include <QLineEdit>
#include <QMetaEnum>
#include <QChildEvent>
#include <QPushButton>
#include <QKeySequence>
#include <QAbstractButton>
#include <QAccessibleEvent>
#include <QGridLayout>
#include <QSpacerItem>
#include <QScrollArea>



/**
 * FIXME:// QMessageBox 无法添加 QCheckbox
 *
 */

enum Button
{
    Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
    Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF, NewButtonMask = 0xFFFFFC00
};

static QMessageBox::StandardButton newButton(int button);
static QMessageDialogOptions::Icon helperIcon(QMessageBox::Icon i);
static QPlatformDialogHelper::StandardButtons helperStandardButtons(MessageBox * q);



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
    QLabel*                                             mLabel;
    QLabel*                                             mInformativeLabel;
    TextEdit*                                           mDetail;                    // qt 显示 label 暂定使用富文本框
    QCheckBox*                                          mCheckbox;                  // qt checkbox
    QLabel*                                             mIconLabel;                 // qt 显示图标
    QDialogButtonBox*                                   mButtonBox;                 // qt 按钮框
    QPushButton*                                        mDetailButton;              // 详细情况按钮
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;
    QScrollArea *contentArea;
//    MessageBoxDetailsText *detailsText;

    QLayout*                                            mMainLayout;                // 主布局
    QLayout*                                            mDetailLayout;              // 详细信息
    QLayout*                                            mButtonLayout;              // 按钮整体布局
    QLayout*                                            mButtonLayout1;             // 复选框按钮
    QLayout*                                            mButtonLayout2;             // 按钮
    QLayout*                                            mContentLayout;             // 内容布局

    QByteArray                                          mMemberToDisconnectOnClose;
    QByteArray                                          mSignalToDisconnectOnClose;
    QPointer<QObject>                                   mReceiverToDisconnectOnClose;


    QString                                             mTipString;                 // 原始的需要展示的文本
    QMessageBox::Icon                                   mIcon;
    QList<QAbstractButton*>                             mCustomButtonList;          // 自定义按钮
    QAbstractButton*                                    mEscapeButton;
    QPushButton*                                        mDefaultButton;
    QAbstractButton*                                    mClickedButton;             // 复选框按钮

    bool                                                mHTML;                      // 文档内容是 html 还是 纯文本
    bool                                                mCompatMode;
    bool                                                mShowDetail;                // 是否显示详细信息
    bool                                                mAutoAddOkButton;

//    QTextEdit* informativeTextEdit;
    QAbstractButton*                                    mDetectedEscapeButton;
    QSharedPointer<QMessageDialogOptions>               mOptions;

private:
    int                             mRadius = 0;
    int                             mIconSize = 24;

    int                             mMinWidth = 380;
    int                             mMinHeight = 142;
    int                             mMaxWidth = 420;
    int                             mMaxHeight = 562;

    int                             mBtnContent = 32;                               // 底部按钮 与 内容框之间的间隔
    int                             mWidgetSpace = 8;

    int                             mMarginLeft = 24;
    int                             mMarginTop = 32;
    int                             mMarginRight = 24;
    int                             mMarginBottom = 24;

    int                             mDetailHeight = 144;
    int                             mDelButton = 24;

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

    setContentsMargins(0, 0, 0, 0);

    setAttribute(Qt::WA_TranslucentBackground);
}

MessageBox::~MessageBox()
{

}

// FIXME:// 不支持此接口
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
        d->mCheckbox->setMaximumWidth(130);
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

QPixmap MessageBox::iconPixmap() const
{
    Q_D(const MessageBox);
    if (d->mIconLabel && d->mIconLabel->pixmap())
        return *d->mIconLabel->pixmap();
    return QPixmap();
}

void MessageBox::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(MessageBox);
    if (!pixmap.isNull())
        d->mIconLabel->setPixmap(pixmap.scaled(d->mIconSize, d->mIconSize));
    d->mIcon = QMessageBox::NoIcon;
}

QString MessageBox::text()
{
    Q_D(const MessageBox);
    return d->mLabel->text();
}

void MessageBox::setText(const QString& text)
{
    Q_D(MessageBox);
    d->mLabel->setText(text);
    d->mLabel->setWordWrap(d->mLabel->textFormat() == Qt::RichText
                          || (d->mLabel->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));
}

Qt::TextFormat MessageBox::textFormat() const
{
    Q_D(const MessageBox);
    return d->mLabel->textFormat();
}

void MessageBox::setTextFormat(Qt::TextFormat format)
{
    Q_D(MessageBox);
    d->mLabel->setTextFormat(format);
    d->mLabel->setWordWrap(format == Qt::RichText
                    || (format == Qt::AutoText && Qt::mightBeRichText(d->mLabel->text())));
}

Qt::TextInteractionFlags MessageBox::textInteractionFlags() const
{
    Q_D(const MessageBox);
    return d->mLabel->textInteractionFlags();
}

void MessageBox::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    Q_D(MessageBox);
    d->mLabel->setTextInteractionFlags(flags);
}

void MessageBox::addButton(QAbstractButton *button, QMessageBox::ButtonRole role)
{
    Q_D(MessageBox);
    if (!button) {
        return;
    }

    removeButton(button);
    d->mButtonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->mCustomButtonList.append(button);
    d->mAutoAddOkButton = false;
}

QPushButton* MessageBox::addButton(const QString &text, QMessageBox::ButtonRole role)
{
    Q_D(MessageBox);
    QPushButton* pushButton = new QPushButton(text);
    addButton(pushButton, role);
    d->updateSize();
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
    d->updateSize();
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
    d->updateSize();
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
            label->setWordWrap(false);
            QPalette palette = label->palette();
            palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
            label->setPalette(palette);
            connect(qApp, &QApplication::paletteChanged, this, [=]() {
                QPalette palette = label->palette();
                palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
                label->setPalette(palette);
            });
            d->mInformativeLabel = label;
        }
        d->mInformativeLabel->setText(text);
    }
}

void MessageBox::setDetailedText(const QString &text)
{
    Q_D(MessageBox);
    if (!text.isEmpty()) {
        d->mDetail = new TextEdit;
        d->mDetail->setText(text);
        d->mDetail->hide();
        if (!d->mDetailButton) {
            d->mDetailButton = new QPushButton(this);
            d->mDetailButton->setText(QObject::tr("Detail"));
        }
    } else {
        d->mDetail = nullptr;
        d->mDefaultButton = nullptr;
    }
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
            setuplayout();
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

    if (d->mDetailButton)
        addButton(d->mDetailButton, QMessageBox::ActionRole);

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
            d->mDetectedEscapeButton->click();
        }
        return;
    }
#endif // QT_CONFIG(shortcut)

#if !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_SHORTCUT)

//#if QT_CONFIG(textedit)
//        if (e == QKeySequence::Copy) {
//            if (d->detailsText && d->detailsText->isVisible() && d->detailsText->copy()) {
//                e->setAccepted(true);
//                return;
//            }
//        } else if (e == QKeySequence::SelectAll && d->detailsText && d->detailsText->isVisible()) {
//            d->detailsText->selectAll();
//            e->setAccepted(true);
//            return;
//        }
//#endif // QT_CONFIG(textedit)

#ifndef QT_NO_SHORTCUT

#endif

#endif // !QT_NO_CLIPBOARD && !QT_NO_SHORTCUT

#ifndef QT_NO_SHORTCUT
    if (!(e->modifiers() & (Qt::AltModifier | Qt::ControlModifier | Qt::MetaModifier))) {
        int key = e->key() & ~Qt::MODIFIER_MASK;
        if (key) {
            const QList<QAbstractButton *> buttons = d->mButtonBox->buttons();
            for (auto *pb : buttons) {
                QKeySequence shortcut = pb->shortcut();
                if (!shortcut.isEmpty() && key == int(shortcut[0] & ~Qt::MODIFIER_MASK)) {
                    pb->animateClick();
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

    Q_UNUSED(event);
    QDialog::paintEvent(event);
}

void MessageBox::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    Q_UNUSED(event);
}

void MessageBox::initHelper(QPlatformMessageDialogHelper* h)
{
    Q_D(MessageBox);

    d->initHelper(h);
}

void MessageBox::setuplayout()
{
    Q_D(MessageBox);
    d->setupLayout();
}


MessageBoxPrivate::MessageBoxPrivate() : mCheckbox(nullptr), mEscapeButton(nullptr), mDefaultButton(nullptr), mClickedButton(nullptr), mCompatMode(false), mAutoAddOkButton(true),
    mDetectedEscapeButton(nullptr), mInformativeLabel(nullptr), mDetail(nullptr), mDetailButton(nullptr), mOptions(QMessageDialogOptions::create())
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

    if (nullptr != mDetail) {
        delete mDetail;
    }

    if (nullptr != mDetailButton) {
        delete mDetailButton;
    }
}

void MessageBoxPrivate::init(const QString &title, const QString &text)
{
    Q_Q(MessageBox);

    mHTML = false;

    mLabel = new QLabel;
    mLabel->setObjectName(QLatin1String("ukui_msgbox_label"));
    mLabel->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, q)));
    mLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    mLabel->setOpenExternalLinks(true);

    contentArea = new QScrollArea;
    contentArea->setBackgroundRole(QPalette::Base);
    contentArea->setAutoFillBackground(true);
    contentArea->setFrameShape(QFrame::NoFrame);

    contentWidget = new QWidget;

    contentLayout = new QVBoxLayout();

    mIconLabel = new QLabel;
    mIconLabel->setObjectName(QLatin1String("ukui_msgbox_icon_label"));
    mIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mIconLabel->setFixedSize(mIconSize, mIconSize);
    mIconLabel->setContentsMargins(0, 0, 0, 0);

    mButtonBox = new QDialogButtonBox;
    mButtonBox->setObjectName(QLatin1String("ukui_msgbox_buttonbox"));
    mButtonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, nullptr, q));
    QObject::connect(mButtonBox, SIGNAL(clicked(QAbstractButton*)), q, SLOT(_q_buttonClicked(QAbstractButton*)));

    if (!text.isEmpty()) {
        q->setText(text);
    }

    q->setModal(true);
    mIcon = QMessageBox::NoIcon;

    Q_UNUSED(title);
}

void MessageBoxPrivate::setupLayout()
{
    Q_Q(MessageBox);

    if (q->layout())
        delete q->layout();

    QSize MaxSize(420, 562 + 6);
    QSize MinSize(380, 142 + 6);

    bool hasIcon = mIconLabel->pixmap() && !mIconLabel->pixmap()->isNull();
    int marge = mMarginLeft + mMarginRight + (hasIcon ? (mIconSize + 8) : 0);

    mLabel->setContentsMargins(0, 0, 0, 0);
    mLabel->setWordWrap(false);
    mLabel->setMinimumWidth(MinSize.width() - marge);
    mLabel->setMaximumWidth(MaxSize.width() - marge);

    if (mInformativeLabel) {
        mInformativeLabel->setWordWrap(false);
        mInformativeLabel->setMinimumWidth(MinSize.width() - marge);
        mInformativeLabel->setMaximumWidth(MaxSize.width() - marge);
    }

    QSize labelSize = mLabel->sizeHint();
    if (labelSize.width() > mLabel->maximumWidth()) {
        mLabel->setMinimumWidth(mLabel->maximumWidth());
        mLabel->setWordWrap(true);
        contentArea->setFixedWidth(MaxSize.width() - marge);
    }

    if (mInformativeLabel) {
        QSize informaSize = mInformativeLabel->sizeHint();
        if (informaSize.width() > mInformativeLabel->maximumWidth()) {
            mInformativeLabel->setMinimumWidth(mInformativeLabel->maximumWidth());
            mInformativeLabel->setWordWrap(true);
            contentArea->setFixedWidth(MaxSize.width() - marge);
        }
    }

    //特殊处理
    if (contentLayout) {
        delete contentLayout;
        contentArea->takeWidget();
    }
    contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(mWidgetSpace);
    contentLayout->addWidget(mLabel);
    if (mInformativeLabel)
        contentLayout->addWidget(mInformativeLabel);
    contentWidget->setLayout(contentLayout);
    contentLayout->activate();
    contentArea->setWidget(contentWidget);

    if (mDetail && !mDetail->isHidden()) {
        mDetail->setFixedSize(MaxSize.width() - mMarginLeft - mMarginRight, mDetailHeight);
        contentArea->setFixedHeight(qMin(contentArea->sizeHint().height(), 276));
        contentArea->setFixedWidth(MaxSize.width() - marge);
    } else {
        contentArea->setFixedHeight(qMin(contentArea->sizeHint().height(), 444));
    }

    QVBoxLayout *grid = new QVBoxLayout;
    grid->setContentsMargins(mMarginLeft, mMarginTop, mMarginRight, mMarginBottom);
    grid->setSpacing(0);

    QGridLayout *contentlayout = new QGridLayout;

    mIconLabel->setContentsMargins(0, 0, 0, 0);
    if (hasIcon) {
        contentlayout->addWidget(mIconLabel, 0, 0, 0, 1, Qt::AlignTop);
        QSpacerItem *indentSpacer = new QSpacerItem(8, mIconSize, QSizePolicy::Fixed, QSizePolicy::Fixed);
        contentlayout->addItem(indentSpacer, 0, 1, 2, 1);
    }
    mIconLabel->setVisible(hasIcon);

    contentlayout->addWidget(contentArea, 0, hasIcon ? 2 : 1, 2, 1);

    grid->addLayout(contentlayout);

    QSpacerItem *buttonSpacer = new QSpacerItem(mBtnContent, mBtnContent, QSizePolicy::Fixed, QSizePolicy::Fixed);
    grid->addItem(buttonSpacer);

//    if (mCheckbox)
//        grid->addWidget(mCheckbox, 2, 0, 1, 1, Qt::AlignLeft);

    grid->addWidget(mButtonBox);

    if (mDetail && !mDetail->isHidden()) {
        QSpacerItem *detailSpacer = new QSpacerItem(1, 24, QSizePolicy::Fixed, QSizePolicy::Fixed);
        grid->addItem(detailSpacer);
        grid->addWidget(mDetail);
    }

    q->setLayout(grid);
    q->setFixedSize(q->sizeHint());

    retranslateStrings();
    updateSize();
}

void MessageBoxPrivate::updateSize()
{
    Q_Q(MessageBox);

    if (!q->isVisible())
        return;
    contentLayout->activate();
    layout->activate();
    QCoreApplication::removePostedEvents(q, QEvent::LayoutRequest);
}

int MessageBoxPrivate::layoutMinimumWidth()
{
    layout->activate();
    return layout->totalMinimumSize().width();
}

void MessageBoxPrivate::_q_buttonClicked(QAbstractButton* button)
{
    Q_Q(MessageBox);

    if (mDetailButton && mDetail && button == mDetailButton) {
        mDetail->setHidden(!mDetail->isHidden());
        mDetailButton->setText(mDetail->isHidden() ? QMessageBox::tr("Show Details...") : QMessageBox::tr("Hide Details..."));
        if (mDetail->isHidden())
            mDetail->hide();
        else
            mDetail->show();
        setupLayout();
    } else {
        setClickedButton(button);
        if (mReceiverToDisconnectOnClose) {
            QObject::disconnect(q, mSignalToDisconnectOnClose, mReceiverToDisconnectOnClose,
                                mMemberToDisconnectOnClose);
            mReceiverToDisconnectOnClose = nullptr;
        }
        mSignalToDisconnectOnClose.clear();
        mMemberToDisconnectOnClose.clear();
    }
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
    if (button0 != 0 && !(button0 & NewButtonMask)) {
        return true;
    }

    if (button1 != 0 && !(button1 & NewButtonMask)) {
        return true;
    }

    if (button2 != 0 && !(button2 & NewButtonMask)) {
        return true;
    }

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

    if (buttons.count() == 2 && mDetailButton) {
        auto idx = buttons.indexOf(mDetailButton);
        if (idx != -1) {
            mDetectedEscapeButton = buttons.at(1 - idx);
            return;
        }
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

void MessageBoxPrivate::retranslateStrings()
{
//    if (detailsButton)
//        detailsButton->setLabel(detailsText->isHidden() ? ShowLabel : HideLabel);
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
//#if QT_CONFIG(textedit)
//    mOptions->setDetailedText(q->detailedText());
//#endif
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
    initDialog();

    if (parent) {
        mMessageBox->move(QPoint((parent->width() - mMessageBox->width()) / 2, (parent->height() - mMessageBox->height()) / 2)
                          + QPoint(parent->x(), parent->y()));
    }

    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(mMessageBox->winId(), hints);

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
    mMessageBox->setInformativeText(options()->informativeText());
    mMessageBox->setDetailedText(options()->detailedText());
    if (options()->text().isNull() && !options()->windowTitle().isNull())
        mMessageBox->setText(options()->windowTitle());
    mMessageBox->d_func()->mOptions = options();

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

    for (QMessageDialogOptions::CustomButton button : options()->customButtons()) {
        QAbstractButton *ab = static_cast<QAbstractButton *>(button.button);
        if (ab) {
            if (!ab->text().isEmpty() && ab->text() == "Show Details..." || ab->text() == "Hide Details...")
                continue;
            ab->setIcon(QIcon());
        }
        mMessageBox->addButton(ab, QMessageBox::ButtonRole(button.role));
    }

    mMessageBox->setuplayout();
}



#include "moc_message-box.cpp"
