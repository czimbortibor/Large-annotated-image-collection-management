#ifndef ABSTRACTFILTER_HPP
#define ABSTRACTFILTER_HPP

#include <QWidget>
#include <QPushButton>

#include "../db/DbContext.hpp"


class AbstractFilter {
public:
	virtual ~AbstractFilter() {}

	virtual AbstractFilter* makeFilter(const DbContext& dbContext) = 0;
    virtual QWidget* makeControl() = 0;
	virtual QPushButton* removeButton() = 0;
};

#endif // ABSTRACTFILTER_HPP
