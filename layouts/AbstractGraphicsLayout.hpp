#ifndef ABSTRACTGRAPHICSLAYOUT_HPP
#define ABSTRACTGRAPHICSLAYOUT_HPP

#include <QGraphicsLayout>
#include <QGraphicsLayoutItem>
#include <QList>


class AbstractGraphicsLayout : public QGraphicsLayout {
public:
	// ------ inherited functions ------
	virtual void setGeometry(const QRectF& geom) = 0;
	virtual int count() const = 0;
	virtual QGraphicsLayoutItem* itemAt(int index) const = 0;
	virtual void removeAt(int index) = 0;
	// ---------------------------------

	/** inserts the item to the front of the list */
	virtual void addItem(QGraphicsLayoutItem* item) = 0;
	/** deletes and removes every element from the layout */
	virtual void clearAll() = 0;
	virtual QList<QGraphicsLayoutItem*>& items() = 0;

protected:
	virtual void insertItem(int index, QGraphicsLayoutItem* item) = 0;
	virtual void setSpacing(Qt::Orientations o, qreal spacing) = 0;
	virtual qreal spacing(Qt::Orientation o) const = 0;
	virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const = 0;
	virtual qreal doLayout(const QRectF& geom, bool applyNewGeometry) const = 0;
	virtual QSizeF minSize(const QSizeF& constraint) const = 0;
	virtual QSizeF prefSize() const = 0;
	virtual QSizeF maxSize() const = 0;
};

#endif // ABSTRACTGRAPHICSLAYOUT_HPP
