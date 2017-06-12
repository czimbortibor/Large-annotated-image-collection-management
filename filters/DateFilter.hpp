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
	DateFilter(const DbContext& dbContext) {}
	~DateFilter() {}

    /**
     * @brief create a new DateFilter for filtering the images between 2 dates
     * @return DateFilter
     */
	DateFilter* makeFilter(const DbContext& dbContext) { return new DateFilter(dbContext); }

    /**
	 * @brief - create a GroupBox containing 2 DateEdit widgets, which will serve as a
	 *			range picker between two dates
	 *		  - create a remove button for the filter
	 * @return 2 QDateEdits and a QPushButton inside a QGroupBox
     */
    QGroupBox* makeControl();

	QPushButton* removeButton() { return _btnRemove; }

    /**
     * @brief calculate the 2 dates in milliseconds (since the epoch) represented in strings
     * @return
     */
    QList<std::string> getDates();

private:
    QGroupBox* _groupBox;
	QPushButton* _btnRemove;
    QDateTimeEdit* _dateEdit1;
    QDateTimeEdit* _dateEdit2;
};

#endif // DATEFILTER_HPP
