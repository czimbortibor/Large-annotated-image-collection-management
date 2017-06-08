#include "DateFilter.hpp"

QGroupBox* DateFilter::makeControl() {
	_dateEdit1 = new QDateTimeEdit();
    _dateEdit1->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit1->setCalendarPopup(true);
	_dateEdit2 = new QDateTimeEdit();
    _dateEdit2->setMinimumDate(QDate(2006, 1, 1));
    _dateEdit2->setCalendarPopup(true);

	QIcon icon(":/icons/resources/icons/x_icon.png");
	_btnRemove = new QPushButton(icon, "");
	_btnRemove->setAutoFillBackground(false);
	_btnRemove->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	_groupBox = new QGroupBox();
    _groupBox->setLayout(new QHBoxLayout);
	_groupBox->layout()->addWidget(_btnRemove);
    _groupBox->layout()->addWidget(_dateEdit1);
    _groupBox->layout()->addWidget(_dateEdit2);
    return _groupBox;
}

QList<std::string> DateFilter::getDates() {
    qint64 date1 = _dateEdit1->dateTime().toMSecsSinceEpoch();
    qint64 date2 = _dateEdit2->dateTime().toMSecsSinceEpoch();
    return QList<std::string>{} << std::to_string(date1) << std::to_string(date2);
}
