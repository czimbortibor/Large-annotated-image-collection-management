#ifndef TEXTFILTER_HPP
#define TEXTFILTER_HPP

#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>

#include "AbstractFilter.hpp"


class TextFilter : public AbstractFilter {
public:
	TextFilter() {}

	TextFilter* makeFilter() { return new TextFilter(); }

	/**
	 * @brief - creates 2 QLineEdits to filter by title and summary
	 *		  - create a remove button for the filter
	 * @return 2 QLineEdits and a QPushButton
	 */
	QGroupBox* makeControl();

	QPushButton* removeButton() { return _btnRemove; }

private:
	QGroupBox* _groupBox;
	QPushButton* _btnRemove;
	QLineEdit* _editTitle;
	QLineEdit* _editSummary;
};

#endif // TEXTFILTER_HPP
