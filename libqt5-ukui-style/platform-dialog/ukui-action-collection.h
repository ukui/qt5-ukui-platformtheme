#ifndef UKUIACTIONCOLLECTION_H
#define UKUIACTIONCOLLECTION_H

#include <QObject>
#include <QAction>
#include <QList>


class UKUIActionCollection : public QObject
{
    Q_OBJECT
public:
    explicit UKUIActionCollection(QObject *parent);
    ~UKUIActionCollection() override;

    QAction *action(const QString &name) const;
    Q_INVOKABLE QAction *addAction(const QString &name, QAction *action);
    void removeAction(QAction *action);
    void addAssociatedWidget(QWidget *widget);
    QAction *takeAction(QAction *action);
    int count() const;
    QList<QAction *> allaction() const;
    QAction *unlistAction(QAction *);

protected Q_SLOTS:
    void _associatedWidgetDestroyed(QObject *obj);

private:
    QMap<QString, QAction *> actionByName;
    QList<QAction *> actions;
    QList<QWidget *> associatedWidgets;
};

#endif // UKUIACTIONCOLLECTION_H
