#ifndef DATEFILTER_HPP
#define DATEFILTER_HPP

#include <chrono>
#include <string>

#include <QDateEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QList>
#include <QDebug>

#include "AbstractFilter.hpp"


class DateFilter : public AbstractFilter {
public:
    DateFilter() {}

    /**
     * @brief create a new DateFilter for filtering the images between 2 dates
     * @return DateFilter
     */
    DateFilter* makeFilter() { return new DateFilter; }

    /**
     * @brief create a GroupBox containing 2 DateEdit widgets, which will serve as a
     * range picker between two dates
     * @return 2 QDateEdits inside a QGroupBox
     */
    QGroupBox* makeControl();

    /**
     * @brief calculate the 2 dates in milliseconds (since the epoch) represented in strings
     * @return
     */
    QList<std::string> getDates();

private:
    QGroupBox* _groupBox;
    QDateTimeEdit* _dateEdit1;
    QDateTimeEdit* _dateEdit2;
};

#endif // DATEFILTER_HPP
