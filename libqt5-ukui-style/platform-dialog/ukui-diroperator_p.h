#ifndef UKUIDIROPERATOR_P_H
#define UKUIDIROPERATOR_P_H

#include "ukui-diroperator.h"

#include <QSplitter>
#include <QUrl>


class UKUIDirOperatorPrivate
{
public:
    explicit UKUIDirOperatorPrivate(UKUIDirOperator *parent)
        : q(parent),
          splitter(nullptr),
          previewWidth(0)
    {

    }

    ~UKUIDirOperatorPrivate()
    {

    }

    UKUIDirOperator * const q;

    QUrl currUrl;

    QSplitter *splitter;
    int previewWidth;
    UKUIDirOperator::Mode mode;
    UKUIDirOperator::FileView viewKind;



    void _u_slotSplitterMoved(int, int);
};

#endif // UKUIDIROPERATOR_P_H
