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

using bsoncxx::builder::stream::document;
using bsoncxx::document::element;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

void TextFilter::on_text_changed(const QString& text) {
	auto query = document{} << "$text" << open_document << "$search" << text.toStdString()
							 << close_document << finalize;
	mongocxx::cursor cursor = _dbContext.feedsCollection.find(query.view());

	QJsonArray results;
	for (const auto& doc : cursor) {
		std::vector<std::string> doc_keys;
		std::transform(std::begin(doc), std::end(doc), std::back_inserter(doc_keys), [](element ele) {
			// key() returns a string_view
			return ele.key().to_string();
		});
		QJsonObject json_obj;
		for (const auto& key : doc_keys) {
			if (doc[key].type() == bsoncxx::type::k_utf8) {
				auto value = doc[key].get_utf8().value.to_string();
				json_obj.insert(QString::fromStdString(key), QString::fromStdString(value));
			}
		results.append(QJsonValue(json_obj));
		}
	}

	emit changed(results);
}

