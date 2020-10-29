#ifndef UKUISTYLE_H
#define UKUISTYLE_H

#include <QStylePlugin>

class UKUIStyle : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "ukui-styletheme-qt.json")

public:
    explicit UKUIStyle(QObject *parent = nullptr);

private:
    QStyle *create(const QString &key) override;
};

#endif // UKUISTYLE_H
