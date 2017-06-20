#include "DateFilter.hpp"


QGroupBox* DateFilter::makeControl() {
	_dateEdit1 = new QDateTimeEdit();
	_dateEdit1->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit1->setCalendarPopup(true);
	_dateEdit1->setDisplayFormat("dd MMM hh:mm yyyy");
	_dateEdit2 = new QDateTimeEdit();
	_dateEdit2->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit2->setCalendarPopup(true);
	_dateEdit2->setDisplayFormat("dd MMM hh:mm yyyy");

	QIcon icon(":/icons/resources/icons/x_icon.png");
	_btnRemove = new QPushButton(icon, "");
	_btnRemove->setAutoFillBackground(false);
	_btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_groupBox = new QGroupBox();
    _groupBox->setLayout(new QHBoxLayout);
	_groupBox->layout()->addWidget(_btnRemove);
    _groupBox->layout()->addWidget(_dateEdit1);
    _groupBox->layout()->addWidget(_dateEdit2);

	connect(_dateEdit1, &QDateTimeEdit::dateTimeChanged, this, &DateFilter::on_date_changed);
	connect(_dateEdit2, &QDateTimeEdit::dateTimeChanged, this, &DateFilter::on_date_changed);

    return _groupBox;
}

void DateFilter::on_date_changed(const QDateTime& datetime) {
	Q_UNUSED(datetime)
	QStringList dates = getDates();

	// emit
}

QStringList DateFilter::getDates() {
	int offset1 = _dateEdit1->dateTime().offsetFromUtc() / 3600;
	int offset2 = _dateEdit2->dateTime().offsetFromUtc() / 3600;
	QString date1 = _dateEdit1->dateTime().toString("ddd, dd MMM hh:mm:ss yyyy");
	date1 = date1 + " +" + QString::number(offset1);
	QString date2 = _dateEdit2->dateTime().toString("ddd, dd MMM hh:mm:ss yyyy");
	date2 = date2 + " +" + QString::number(offset2);
	//QString date2 = _dateEdit2->dateTime().toTimeSpec(Qt::OffsetFromUTC).toString(Qt::ISODate);
	return QStringList{} << date1 << date2;
}
