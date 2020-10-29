#ifndef UKUISTYLEPLUGIN_H
#define UKUISTYLEPLUGIN_H

#include <QStylePlugin>
#include <private/qfusionstyle_p.h>
#include <black-list.h>
#include <QApplication>
#include "ukui-style.h"


class UKUIStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "ukui-systemstyle-qt.json")

public:
    explicit UKUIStylePlugin(QObject *parent = nullptr);

private:
    QStyle *create(const QString &key) override;
    const QStringList blackList();
};

#endif // UKUISTYLEPLUGIN_H
