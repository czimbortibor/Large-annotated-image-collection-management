#include "TextFilter.hpp"

QGroupBox* TextFilter::makeControl() {
	_editTitle = new QLineEdit();
	_editTitle->setPlaceholderText("search by title");
	_editSummary = new QLineEdit();
	_editSummary->setPlaceholderText("search by summary");

	_groupBox = new QGroupBox();
	_groupBox->setLayout(new QVBoxLayout());
	QIcon icon(":/icons/resources/icons/x_icon.png");
	_btnRemove = new QPushButton(icon, "");
	_btnRemove->setAutoFillBackground(false);
	_btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_groupBox->layout()->addWidget(_btnRemove);
	_groupBox->layout()->addWidget(_editTitle);
	_groupBox->layout()->addWidget(_editSummary);
	return _groupBox;
}
