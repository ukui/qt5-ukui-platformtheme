#ifndef MESSAGEBOXHELPER_H
#define MESSAGEBOXHELPER_H

#include <QObject>

class MessageboxHelper : public QObject
{
    Q_OBJECT
public:
    explicit MessageboxHelper(QObject *parent = nullptr);

    void registerWidget(QWidget *widget);
    void unregisterWidget(const QWidget *widget);

signals:

};

#endif // MESSAGEBOXHELPER_H
