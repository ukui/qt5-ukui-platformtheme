#ifndef UKUIDIROPERATOR_H
#define UKUIDIROPERATOR_H

#include <QObject>
#include <QWidget>
#include <QUrl>

class UKUIDirOperatorPrivate;
class UKUIDirOperator : public QWidget
{
//    Q_OBJECT
public:
    enum Mode {
        File         = 1,
        Directory    = 2,
        Files        = 4,
        ExistingOnly = 8,
        LocalOnly    = 16,
        ModeMax      = 65536
    };

    enum FileView {
        Default         = 0,
        Simple          = 1,
        Detail          = 2,
        SeparateDirs    = 4,
        PreviewContents = 8,
        PreviewInfo     = 16,
        Tree            = 32,
        DetailTree      = 64,
        FileViewMax     = 65536
    };

    explicit UKUIDirOperator(const QUrl &urlName  = QUrl(), QWidget *parent = nullptr);
    ~UKUIDirOperator();

private:
    UKUIDirOperatorPrivate *const d;

    Q_PRIVATE_SLOT(d, void _u_slotSplitterMoved(int, int))
};

#endif // UKUIDIROPERATOR_H
