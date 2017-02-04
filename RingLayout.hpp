#ifndef RINGLAYOUT_HPP
#define RINGLAYOUT_HPP

#include <QGraphicsLayout>
#include <QWidget>
#include <qmath.h>
#include <QDebug>

class RingLayout : public QGraphicsLayout {
public:
	RingLayout();
	/** inserts the item to the front of the list */
	void addItem(QGraphicsLayoutItem* item) { insertItem(-1, item); }
	void setSpacing(Qt::Orientations orientation, qreal spacing);
	qreal spacing(Qt::Orientation orientation) const { return _spacing[int(orientation) - 1]; }

	void setNrOfPetals(int value) { _nrOfPetals = value; }
	void setRadius(qreal value) { _radius = value; }

	// inherited functions
	void setGeometry(const QRectF& geometry) Q_DECL_OVERRIDE;
	int count() const Q_DECL_OVERRIDE { return _items.count(); }
	QGraphicsLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE { return _items.value(index); }
	void removeAt(int index) Q_DECL_OVERRIDE { _items.removeAt(index); invalidate(); }
	// --------------------

	/** deletes and removes every element from the layout */
	void clearAll();

protected:
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const Q_DECL_OVERRIDE;

private:
	void insertItem(int index, QGraphicsLayoutItem* item);
	qreal doLayout(const QRectF& geometry, bool applyNewGeometry) const;
	QSizeF minSize(const QSizeF& constraint) const;
	QSizeF prefSize() const;
	QSizeF maxSize() const;

	QList<QGraphicsLayoutItem*> _items;
	qreal _spacing[2];
	/** how many images should be on the ring */
	int _nrOfPetals;
	/** radius of the circle on which the images will be positioned */
	qreal _radius;

};

#endif // RINGLAYOUT_HPP
