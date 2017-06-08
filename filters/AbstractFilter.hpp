#ifndef ABSTRACTFILTER_HPP
#define ABSTRACTFILTER_HPP

#include <QWidget>
#include <QPushButton>


class AbstractFilter {
public:
    AbstractFilter() {}
    virtual AbstractFilter* makeFilter() = 0;
    virtual QWidget* makeControl() = 0;
	virtual QPushButton* removeButton() = 0;
};

#endif // ABSTRACTFILTER_HPP
