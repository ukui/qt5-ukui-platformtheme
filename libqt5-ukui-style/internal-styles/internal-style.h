#ifndef INTERNALSTYLE_H
#define INTERNALSTYLE_H

#include <QProxyStyle>

/*!
 * \brief The InternalStyle class
 * \details
 * This class is a interface type class. It is desgined as an extension of UKUI theme
 * frameworks. Applications which use internal style means that there will be no effect
 * when system theme switched, for example, from fusion to ukui-white.
 * But an internal style usually should response the palette settings changed for
 * keeping the unity as a desktop environment theme's extensions.
 *
 * The typical example which implement the internal style is MPSStyle.
 */
class InternalStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit InternalStyle(QStyle *parentStyle = nullptr);
    explicit InternalStyle(const QString parentStyleName);

signals:
    void useSystemStylePolicyChanged(bool use);

public slots:
    virtual void setUseSystemStyle(bool use);
};

#endif // INTERNALSTYLE_H
