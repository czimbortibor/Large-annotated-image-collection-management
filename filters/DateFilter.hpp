#ifndef DATEFILTER_HPP
#define DATEFILTER_HPP

#include <QDateEdit>

#include "AbstractFilter.hpp"


class DateFilter : public AbstractFilter {
public:
    DateFilter() {}

    DateFilter* makeFilter() { return new DateFilter; }
    QDateEdit* makeControl();

};

#endif // DATEFILTER_HPP
