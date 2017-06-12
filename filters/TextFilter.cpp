#include "TextFilter.hpp"


QGroupBox* TextFilter::makeControl() {
	_editTitle = new QLineEdit();
	_editTitle->setPlaceholderText("search by title");
	_editSummary = new QLineEdit();
	_editSummary->setPlaceholderText("search by summary");
	connect(_editTitle, &QLineEdit::textChanged, this, &TextFilter::on_text_changed);

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

void TextFilter::on_text_changed(const QString& newText) {
	/*auto res = _dbConnection->list_databases();
	for (auto&& doc : res) {
		std::cout << bsoncxx::to_json(doc) << std::flush;
	}*/
	_dbContext.filterTitle(newText.toStdString());
}
