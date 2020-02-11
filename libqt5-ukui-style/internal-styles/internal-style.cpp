#include "internal-style.h"

InternalStyle::InternalStyle(QStyle *parentStyle) : QProxyStyle (parentStyle)
{

}

InternalStyle::InternalStyle(const QString parentStyleName) : QProxyStyle(parentStyleName)
{

}

void InternalStyle::setUseSystemStyle(bool use)
{
    Q_EMIT useSystemStylePolicyChanged(use);
}
