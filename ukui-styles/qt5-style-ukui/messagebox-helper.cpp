#include "messagebox-helper.h"

#include <QWidget>

MessageboxHelper::MessageboxHelper(QObject *parent) : QObject(parent)
{

}

void MessageboxHelper::registerWidget(QWidget *widget)
{
    widget->setBackgroundRole(QPalette::Base);
    widget->setAutoFillBackground(true);
}

void MessageboxHelper::unregisterWidget(const QWidget *widget)
{

}
