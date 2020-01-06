#ifndef QT5UKUISTYLEPLUGIN_H
#define QT5UKUISTYLEPLUGIN_H

#include <QStylePlugin>

class Qt5UKUIStylePlugin : public QStylePlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "qt5-style-ukui.json")
#endif // QT_VERSION >= 0x050000

public:
    Qt5UKUIStylePlugin(QObject *parent = 0);

    QStyle *create(const QString &key) override;
};

#endif // QT5UKUISTYLEPLUGIN_H
