#ifndef SELECTEFFECT_HPP
#define SELECTEFFECT_HPP

#include <QGraphicsEffect>
#include <QPainter>


class SelectEffect : public QGraphicsEffect {
	Q_OBJECT

public:
	SelectEffect(qreal offset = 1.2) {
		_color = QColor(0, 0, 255, 200);
		_offset = QPointF(offset, offset);
	}

	QRectF boundingRectFor(const QRectF& sourceRect) const {
		return sourceRect.adjusted(-_offset.x(), -_offset.y(), _offset.x(), _offset.y());
	}

	void setColor(const QColor& color) { _color = color; }
	void setOffset(const QPointF& offset) { _offset = offset; }

protected:
	void draw(QPainter* painter) {
		QPoint offset;
		QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
		QRectF bound = boundingRectFor(pixmap.rect());
		painter->save();
		painter->setPen(Qt::NoPen);
		painter->setBrush(_color);
		QPointF point(offset.x() - _offset.x(), offset.y() - _offset.y());
		bound.moveTopLeft(point);
		painter->drawRoundedRect(bound, 10, 10, Qt::RelativeSize);
		painter->drawPixmap(offset, pixmap);
		painter->restore();
	}

private:
	QPointF _offset;
	QColor _color;
};

#endif // SELECTEFFECT_HPP
