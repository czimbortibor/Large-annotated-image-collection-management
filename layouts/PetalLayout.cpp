#include "PetalLayout.hpp"

PetalLayout::PetalLayout() {
	_spacing[0] = 1;
	_spacing[1] = 1;

	QSizePolicy sizePol = sizePolicy();
	// should the widget's preferred height depend on its width
	sizePol.setHeightForWidth(true);
	setSizePolicy(sizePol);
}

void PetalLayout::insertItem(int index, QGraphicsLayoutItem* item) {
	QGraphicsLayout::addChildLayoutItem(item);
	//item->setParentLayoutItem(this);
	if (index > _items.count()) {
		index = _items.count();
	}
	_items.insert(index, item);
	invalidate();
}

void PetalLayout::clearAll() {
	// calls delete on every element
	qDeleteAll(_items.begin(), _items.end());
	// removes the elements from the container
	_items.clear();
	invalidate();
}

void PetalLayout::setSpacing(Qt::Orientations orientation, qreal spacing) {
	if (orientation & Qt::Horizontal) {
		_spacing[0] = spacing;
	}
	if (orientation & Qt::Vertical) {
		_spacing[1] = spacing;
	}
}

void PetalLayout::setGeometry(const QRectF& geometry) {
	QGraphicsLayout::setGeometry(geometry);
	doLayout(geometry, true);
}

qreal PetalLayout::doLayout(const QRectF& geometry, bool applyNewGeometry) const {
	qreal left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	const qreal maxWidth = geometry.width() - left - right;

	// scene's center
	qreal x = 0;
	qreal y = 0;
	qreal maxRowHeight = 0;
	QSizeF prefSize;
	qreal next_x = 0;

	qreal degree = 360.0 / _nrOfPetals;
	qreal radian = qDegreesToRadians(degree);
	qreal r = 0;

	/** minimal horizontal distance between two flowers */
	qreal minXDist;
	/** minimal vertical distance between two flowers */
	qreal minYDist = 2.5 * _radius;
	qreal dx;
	qreal dy;

	for (int i = 0; i < _items.size(); ++i) {
		QGraphicsLayoutItem* item = _items.at(i);
		prefSize = item->effectiveSizeHint(Qt::PreferredSize);

		// center image
		if (i == 0 || i % (_nrOfPetals + 1) == 0) {
			if (i == 0) {
				minXDist = _radius;
			}
			else {
				minXDist = 3 * _radius + spacing(Qt::Horizontal);
			}
			maxRowHeight = (maxRowHeight > prefSize.height() + minYDist) ? maxRowHeight : prefSize.height() + minYDist;
			next_x = x + minXDist; /*prefSize.width() +;*/
			if (next_x > maxWidth) {
				prefSize.setWidth(maxWidth);
				next_x = _radius;
				y += maxRowHeight;
				//maxRowHeight = 0;
			}
			x = next_x;
			if (applyNewGeometry) {
				item->setGeometry(QRectF(QPointF(x, y), prefSize));
			}
		}
		// images on the ring/petals
		else {
			dx = x + _radius * qCos(r);
			dy = y + _radius * qSin(r);
			if (applyNewGeometry) {
				item->setGeometry(QRectF(QPointF(dx, dy), prefSize));
			}
			r += radian;
		}
	}

	return top + y + maxRowHeight + bottom;
}

QSizeF PetalLayout::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const {
	QSizeF hintedSize = constraint;
	switch (which) {
		case Qt::PreferredSize:
			hintedSize = prefSize();
			break;
		case Qt::MinimumSize:
			hintedSize = minSize(constraint);
			break;
		case Qt::MaximumSize:
			hintedSize = maxSize();
			break;
		default:
			break;
	}
	return hintedSize;
}

QSizeF PetalLayout::minSize(const QSizeF& constraint) const {
	QSizeF size(0, 0);
	qreal left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	 // height for width
	if (constraint.width() >= 0) {
		const qreal height = doLayout(QRectF(QPointF(0,0), constraint), false);
		size = QSizeF(constraint.width(), height);
	}
	else {
		for (const auto& item : _items) {
			// returns a size holding the maximum width and height of this size and the given parameter
			size = size.expandedTo(item->effectiveSizeHint(Qt::MinimumSize));
		}
		size += QSize(left + right, top + bottom);
	}
	return size;
}

QSizeF PetalLayout::maxSize() const {
	qreal totalWidth = 0;
	qreal totalHeight = 0;
	for (const auto& item : _items) {
		if (totalWidth > 0) {
			totalWidth += spacing(Qt::Horizontal);
		}
		if (totalHeight > 0) {
			totalHeight += spacing(Qt::Vertical);
		}
		QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
		totalWidth += pref.width();
		totalHeight += pref.height();
	}

	qreal left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	return QSizeF(left + totalWidth + right, top + totalHeight + bottom);
}

QSizeF PetalLayout::prefSize() const {
	qreal left, right;
	getContentsMargins(&left, 0, &right, 0);

	qreal maxh = 0;
	qreal totalWidth = 0;
	for (const auto& item : _items) {
		if (totalWidth > 0) {
			totalWidth += spacing(Qt::Horizontal);
		}
		QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
		totalWidth += pref.width();
		maxh = qMax(maxh, pref.height());
	}
	maxh += spacing(Qt::Vertical);

	const qreal goldenAspectRatio = 1.61803399;
	qreal width = qSqrt(totalWidth * maxh * goldenAspectRatio) + left + right;

	return minSize(QSizeF(width, -1));
}
