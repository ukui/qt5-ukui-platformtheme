#include "ukui-action-collection.h"

UKUIActionCollection::UKUIActionCollection(QObject *parent)
    : QObject(parent)
{

}

UKUIActionCollection::~UKUIActionCollection()
{

}



QAction *UKUIActionCollection::action(const QString &name) const
{
    QAction *action = nullptr;

    if (!name.isEmpty()) {
        action = actionByName.value(name);
    }

    return action;
}

QAction *UKUIActionCollection::addAction(const QString &name, QAction *action)
{
    if (!action) {
        return action;
    }

    const QString objectName = action->objectName();
    QString indexName = name;

    if (indexName.isEmpty()) {
        // No name provided. Use the objectName.
        indexName = objectName;
    } else {
        // A name was provided. Check against objectName.
        if ((!objectName.isEmpty()) && (objectName != indexName)) {
            if (actionByName.contains(objectName))
                indexName = objectName;
        }
        // Set the new name
        action->setObjectName(indexName);
    }
    // No name provided and the action had no name. Make one up. This will not
    // work when trying to save shortcuts. Both local and global shortcuts.
    if (indexName.isEmpty()) {
        indexName = QString::asprintf("unnamed-%p", (void *)action);
        action->setObjectName(indexName);
    }
    // From now on the objectName has to have a value. Else we cannot safely
    // remove actions.
    Q_ASSERT(!action->objectName().isEmpty());

    // look if we already have THIS action under THIS name ;)
    if (actionByName.value(indexName, nullptr) == action) {
        // This is not a multi map!
        return action;
    }
    // Check if we have another action under this name and remove it.
    if (QAction *oldAction = actionByName.value(indexName)) {
        takeAction(oldAction);
    }

    // Check if we have this action under a different name.
    // Not using  takeAction because we don't want to remove it from categories,
    // and because it has the new name already.
    const int oldIndex = actions.indexOf(action);
    if (oldIndex != -1) {
        actionByName.remove(actionByName.key(action));
        actions.removeAt(oldIndex);
    }

    // Add action to our lists.
    actionByName.insert(indexName, action);
    actions.append(action);

    for (QWidget *widget : qAsConst(associatedWidgets))
        widget->addAction(action);

    return action;
}

void UKUIActionCollection::removeAction(QAction *action)
{
    delete takeAction(action);
}

void UKUIActionCollection::addAssociatedWidget(QWidget *widget)
{
    if (!associatedWidgets.contains(widget)) {
        widget->addActions(allaction());

        associatedWidgets.append(widget);
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(_associatedWidgetDestroyed(QObject*)));
    }
}

QAction *UKUIActionCollection::takeAction(QAction *action)
{
    if (!unlistAction(action)) {
        return nullptr;
    }

    // Remove the action from all widgets
    for (QWidget *widget : qAsConst(associatedWidgets)) {
        widget->removeAction(action);
    }

    action->disconnect(this);
    return action;
}

int UKUIActionCollection::count() const
{
    return actions.count();
}

QList<QAction *> UKUIActionCollection::allaction() const
{
    return actions;
}

/*
 * Remove a action from our internal bookkeeping.
 * Returns a nullptr if the action doesn't belong to us
*/
QAction *UKUIActionCollection::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called with an QObject formerly known as a QAction
    //   during _k_actionDestroyed(). So don't do fancy stuff here that needs a
    //   real QAction!

    // Get the index for the action
    int index = actions.indexOf(action);

    // Action not found.
    if (index == -1) {
        return nullptr;
    }

    // An action collection can't have the same action twice.
    Q_ASSERT(actions.indexOf(action, index + 1) == -1);

    // Get the actions name
    const QString name = action->objectName();

    // Remove the action
    actionByName.remove(name);
    actions.removeAt(index);
    return action;
}

void UKUIActionCollection::_associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.removeAll(static_cast<QWidget *>(obj));
}
