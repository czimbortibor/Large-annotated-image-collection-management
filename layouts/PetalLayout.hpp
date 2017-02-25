#ifndef RINGLAYOUT_HPP
#define RINGLAYOUT_HPP

#include <qmath.h>

#include <QWidget>
#include <QDebug>

#include "AbstractGraphicsLayout.hpp"


class PetalLayout : public AbstractGraphicsLayout {
public:
	explicit PetalLayout();

	void setSpacing(Qt::Orientations orientation, qreal spacing) Q_DECL_OVERRIDE;
	qreal spacing(Qt::Orientation orientation) const Q_DECL_OVERRIDE { return _spacing[int(orientation) - 1]; }
	void setGeometry(const QRectF& geometry) Q_DECL_OVERRIDE;
	int count() const Q_DECL_OVERRIDE { return _items.count(); }
	QGraphicsLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE { return _items.at(index); }
	void removeAt(int index) Q_DECL_OVERRIDE { _items.removeAt(index); invalidate(); }

	void addItem(QGraphicsLayoutItem* item) Q_DECL_OVERRIDE { insertItem(-1, item); }
	void clearAll() Q_DECL_OVERRIDE;
	void setNrOfPetals(int value) { _nrOfPetals = value; }
	void setRadius(qreal value) { _radius = value; }
	QList<QGraphicsLayoutItem*>& items() Q_DECL_OVERRIDE { return _items; }

protected:
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const Q_DECL_OVERRIDE;

private:
	void insertItem(int index, QGraphicsLayoutItem* item) Q_DECL_OVERRIDE;
	qreal doLayout(const QRectF& geometry, bool applyNewGeometry) const Q_DECL_OVERRIDE;
	QSizeF minSize(const QSizeF& constraint) const Q_DECL_OVERRIDE;
	QSizeF prefSize() const Q_DECL_OVERRIDE;
	QSizeF maxSize() const Q_DECL_OVERRIDE;

	QList<QGraphicsLayoutItem*> _items;
	qreal _spacing[2];
	/** how many images should be on the ring */
	int _nrOfPetals = 7;
	/** radius of the circle on which the images will be positioned */
	qreal _radius = 100;

};

#endif // RINGLAYOUT_HPP
