#ifndef TEXTFILTER_HPP
#define TEXTFILTER_HPP

#include <QObject>
#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>

#include "AbstractFilter.hpp"


class TextFilter : public QObject, public AbstractFilter {
	Q_OBJECT
public:
	TextFilter(const DbContext& dbContext) {
		_dbContext = dbContext;
		_dbConnection = DbContext::MongoAccess::instance().get_connection();
	}
	~TextFilter() = default;

	TextFilter* makeFilter(const DbContext& dbContext) { return new TextFilter(dbContext); }

	/**
	 * @brief - creates 2 QLineEdits to filter by title and summary
	 *		  - create a remove button for the filter
	 * @return 2 QLineEdits and a QPushButton
	 */
	QGroupBox* makeControl();

	QPushButton& removeButton() { return *_btnRemove; }

	QString getText() { return _editField->text(); }

signals:
	void changed(const QJsonArray& results);

private slots:
	void on_text_changed(const QString& text);

private:
	DbContext _dbContext;
	mongocxx::pool::entry _dbConnection;
	QGroupBox* _groupBox;
	QPushButton* _btnRemove;
	QLineEdit* _editField;
};

#endif // TEXTFILTER_HPP
