#ifndef UKUISTYLE_H
#define UKUISTYLE_H

#include <private/qfusionstyle_p.h>



class UKUIStyle : public QFusionStyle
{

public:
    enum StyleType {
        udefault,
        udark,
        ulight
    };

    UKUIStyle(StyleType type = udefault);

private:
    StyleType m_type;
};

#endif // UKUISTYLE_H
