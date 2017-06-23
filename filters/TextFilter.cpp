#include "TextFilter.hpp"


QGroupBox* TextFilter::makeControl() {
	_editField = new QLineEdit();
	_editField->setPlaceholderText("search in title and summary");
	_editField->setMinimumWidth(175);

	connect(_editField, &QLineEdit::textChanged, this, &TextFilter::on_text_changed);

	_groupBox = new QGroupBox();
	_groupBox->setLayout(new QVBoxLayout());
	QIcon icon(":/icons/resources/icons/x_icon.png");
	_btnRemove = new QPushButton(icon, "");
	_btnRemove->setAutoFillBackground(false);
	_btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_groupBox->layout()->addWidget(_btnRemove);
	_groupBox->layout()->addWidget(_editField);
	return _groupBox;
}

void TextFilter::on_text_changed(const QString& text) {
	QJsonArray results = _dbContext.queryText(text);
	emit changed(results);
}

