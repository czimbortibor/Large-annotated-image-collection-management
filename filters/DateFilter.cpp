#include "DateFilter.hpp"


QGroupBox* DateFilter::makeControl() {
	_dateEdit1 = new QDateTimeEdit();
	_dateEdit1->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit1->setCalendarPopup(true);
	_dateEdit1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_dateEdit2 = new QDateTimeEdit();
	_dateEdit2->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit2->setCalendarPopup(true);
	_dateEdit2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	_dateEdit1->setDisplayFormat("yyyy/M/dd hh:mm");
	_dateEdit2->setDisplayFormat("yyyy/M/dd hh:mm");

	QIcon icon(":/icons/resources/icons/x_icon.png");
	_btnRemove = new QPushButton(icon, "");
	_btnRemove->setAutoFillBackground(false);
	_btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	_btnApply = new QPushButton("Apply");
	_btnApply->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(_btnApply, &QPushButton::clicked, this, &DateFilter::on_query_dates);

	_groupBox = new QGroupBox();
	QGridLayout* grid = new QGridLayout();
	grid->addWidget(_btnRemove, 0, 0);
	grid->addWidget(_btnApply, 0, 1);
	grid->addWidget(_dateEdit1, 1, 0);
	grid->addWidget(_dateEdit2, 1, 1);
	_groupBox->setLayout(grid);
    return _groupBox;
}

void DateFilter::on_query_dates() {
	QStringList dates = getDates();
	QJsonArray results = _dbContext.queryDateRange(dates);
	emit datesChanged(results);
}

QStringList DateFilter::getDates() {
	/*int offset1 = _dateEdit1->dateTime().offsetFromUtc() / 3600;
	int offset2 = _dateEdit2->dateTime().offsetFromUtc() / 3600;
	*/
	//QString date1 = _dateEdit1->dateTime().toString("ddd, dd MMM hh:mm:ss yyyy");
	//date1 = date1 + " +" + QString::number(offset1);
	//QString date2 = _dateEdit2->dateTime().toString("ddd, dd MMM hh:mm:ss yyyy");
	//date2 = date2 + " +" + QString::number(offset2);
	//QString date2 = _dateEdit2->dateTime().toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate);

	QString date1 = _dateEdit1->dateTime().toString(Qt::ISODate);
	QString date2 = _dateEdit2->dateTime().toString(Qt::ISODate);
	return QStringList{} << date1 << date2;
}
