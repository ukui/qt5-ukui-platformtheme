#ifndef PROXYSTYLEPLUGIN_H
#define PROXYSTYLEPLUGIN_H

#include <QStylePlugin>

namespace UKUI {

class ProxyStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "ukui-style.json")
public:
    ProxyStylePlugin() {}

    QStyle *create(const QString &key) override;

signals:

public slots:
};

}

#endif // PROXYSTYLEPLUGIN_H
