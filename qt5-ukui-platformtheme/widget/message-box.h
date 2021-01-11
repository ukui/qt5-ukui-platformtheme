#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QIcon>
#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <qpa/qplatformdialoghelper.h>

class QLabel;
class QAbstractButton;
class MessageBoxHelper;
class MessageBoxPrivate;
class MessageBoxOptionsPrivate;

typedef QMessageBox::Icon               Icon;
typedef QMessageBox::StandardButtons    StandardButtons;

class MessageBox : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(Icon mIcon READ icon WRITE setIcon)
    Q_PROPERTY(QString mText READ text WRITE setText)
    Q_PROPERTY(QPixmap mIconPixmap READ iconPixmap WRITE setIconPixmap)
    Q_PROPERTY(QString mInformativeText READ informativeText WRITE setInformativeText)
    Q_PROPERTY(StandardButtons mStandardButtons READ standardButtons WRITE setStandardButtons)
    friend MessageBoxHelper;
public:
    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox();

    void setCheckBox(QCheckBox *cb);
    QCheckBox* checkBox() const;

    QMessageBox::Icon icon ();
    void setIcon (QMessageBox::Icon icon);

    QString text();
    void setText (const QString& text);

    QString informativeText() const;
    void setInformativeText(const QString &text);

    QPixmap iconPixmap() const;
    void setIconPixmap(const QPixmap &pixmap);

    QString buttonText(int button) const;
    void setButtonText(int button, const QString &text);

    void removeButton(QAbstractButton *button);
    QPushButton* addButton(QMessageBox::StandardButton button);
    void addButton(QAbstractButton *button, QMessageBox::ButtonRole role);
    QPushButton* addButton(const QString &text, QMessageBox::ButtonRole role);

    QAbstractButton* button (QMessageBox::StandardButton which) const;

    QList<QAbstractButton*> buttons() const;
    QMessageBox::ButtonRole buttonRole(QAbstractButton *button) const;

    QMessageBox::StandardButtons standardButtons() const;
    void setStandardButtons(QMessageBox::StandardButtons buttons);
    QMessageBox::StandardButton standardButton(QAbstractButton *button) const;

    QPushButton* defaultButton() const;
    void setDefaultButton(QPushButton *button);
    void setDefaultButton(QMessageBox::StandardButton button);

    QAbstractButton* escapeButton() const;
    void setEscapeButton(QAbstractButton *button);
    void setEscapeButton(QMessageBox::StandardButton button);

    QAbstractButton* clickedButton() const;

    void setWindowTitle(const QString &title);
    void setWindowModality(Qt::WindowModality windowModality);

    static QPixmap standardIcon(QMessageBox::Icon icon);

protected:
    bool event(QEvent *e) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent (QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initHelper(QPlatformMessageDialogHelper*);

Q_SIGNALS:
    void buttonClicked(QAbstractButton* button);

private:
    Q_DISABLE_COPY(MessageBox)
    Q_DECLARE_PRIVATE(MessageBox)

    Q_PRIVATE_SLOT(d_func(), void _q_buttonClicked(QAbstractButton*))
    Q_PRIVATE_SLOT(d_func(), void _q_clicked(QPlatformDialogHelper::StandardButton, QPlatformDialogHelper::ButtonRole))
};

class MessageBoxHelper : public QPlatformMessageDialogHelper
{
    Q_OBJECT
public:
    explicit MessageBoxHelper();
    ~MessageBoxHelper() override;

    virtual void exec() override;
    virtual void hide() override;
    virtual bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;

Q_SIGNALS:
    void clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role);

private:
    void initDialog ();

private:
    MessageBox*                                 mMessageBox = nullptr;
};

#endif // MESSAGEBOX_H

#define QT_REQUIRE_VERSION(argc, argv, str) { QString s = QString::fromLatin1(str);\
QString sq = QString::fromLatin1(qVersion()); \
if ((sq.section(QChar::fromLatin1('.'),0,0).toInt()<<16)+\
(sq.section(QChar::fromLatin1('.'),1,1).toInt()<<8)+\
sq.section(QChar::fromLatin1('.'),2,2).toInt()<(s.section(QChar::fromLatin1('.'),0,0).toInt()<<16)+\
(s.section(QChar::fromLatin1('.'),1,1).toInt()<<8)+\
s.section(QChar::fromLatin1('.'),2,2).toInt()) { \
if (!qApp){ \
    new QApplication(argc,argv); \
} \
QString s = QApplication::tr("Executable '%1' requires Qt "\
 "%2, found Qt %3.").arg(qAppName()).arg(QString::fromLatin1(\
str)).arg(QString::fromLatin1(qVersion())); QMessageBox::critical(0, QApplication::tr(\
"Incompatible Qt Library Error"), s, QMessageBox::Abort, 0); qFatal("%s", s.toLatin1().data()); }}
