#ifndef DATEFILTER_HPP
#define DATEFILTER_HPP

#include <chrono>
#include <string>

#include <QObject>
#include <QDateEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QList>

#include "AbstractFilter.hpp"


class DateFilter : public QObject, public AbstractFilter {
	Q_OBJECT
public:
	DateFilter(const DbContext& dbContext) {
		_dbContext = dbContext;
		_dbConnection = DbContext::MongoAccess::instance().get_connection();
	}
	~DateFilter() = default;

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

	QPushButton& removeButton() { return *_btnRemove; }

	QPushButton& applyButton() { return *_btnApply; }

    /**
     * @brief calculate the 2 dates in milliseconds (since the epoch) represented in strings
     * @return
     */
	QStringList getDates();

signals:
	void datesChanged(const QJsonArray& results);

private slots:
	void on_query_dates();

private:
	DbContext _dbContext;
	mongocxx::pool::entry _dbConnection;
    QGroupBox* _groupBox;
	QPushButton* _btnRemove;
	QPushButton* _btnApply;
    QDateTimeEdit* _dateEdit1;
    QDateTimeEdit* _dateEdit2;
};

#endif // DATEFILTER_HPP
