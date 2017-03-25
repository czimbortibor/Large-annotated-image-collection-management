#include "DateFilter.hpp"

QDateEdit* DateFilter::makeControl() {
    QDateEdit* dateEdit = new QDateEdit;
    dateEdit->setMinimumDate(QDate(2006, 1, 1));
    dateEdit->setCalendarPopup(true);
    return dateEdit;
}
