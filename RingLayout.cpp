#include "RingLayout.hpp"

RingLayout::RingLayout() {
	_spacing[0] = 1;
	_spacing[1] = 1;

	QSizePolicy sizePol = sizePolicy();
	// should the widget's preferred height depend on its width
	sizePol.setHeightForWidth(true);
	setSizePolicy(sizePol);
}

void RingLayout::insertItem(int index, QGraphicsLayoutItem* item) {
	item->setParentLayoutItem(this);
	if (index > _items.count()) {
		index = _items.count();
	}
	_items.insert(index, item);
	invalidate();
}

void RingLayout::setSpacing(Qt::Orientations orientation, qreal spacing) {
	if (orientation & Qt::Horizontal) {
		_spacing[0] = spacing;
	}
	if (orientation & Qt::Vertical) {
		_spacing[1] = spacing;
	}
}

void RingLayout::setGeometry(const QRectF& geometry) {
	QGraphicsLayout::setGeometry(geometry);
	doLayout(geometry, true);
}

qreal RingLayout::doLayout(const QRectF& geometry, bool applyNewGeometry) const {
	qreal left, top, right, bottom;
	getContentsMargins(&left, &top, &right, &bottom);
	const qreal maxWidth = geometry.width() - left - right;

	qreal x = 0;
	qreal y = 0;
	qreal maxRowHeight = 0;
	QSizeF prefSize;
	qreal next_x = 0;

	qreal degree = 2.0 * M_PI / _items.size();
	qreal radian = qDegreesToRadians(degree);
	qreal r = 0;

	qreal radius;
	qreal dx = x + radius * qCos(r);
	qreal dy = y + radius * qSin(r);

	/*int n = _items.size();
	QGraphicsLayoutItem* item;
	if (n == 1 || n % 7 == 0) {
		if (n == 1) {
			item = _items.at(0);
		} else {
			item = _items.at(n-1);
		}

		prefSize = item->effectiveSizeHint(Qt::PreferredSize);
		radius = prefSize.width() - left - right / 2;
		maxRowHeight = (maxRowHeight > prefSize.height()) ? maxRowHeight : prefSize.height();
		next_x = x + prefSize.width();
		if (next_x > maxWidth) {
			if (x == 0) {
				prefSize.setWidth(maxWidth);
			} else {
				x = 0;
				next_x = prefSize.width();
			}
			y += maxRowHeight + spacing(Qt::Vertical);
			maxRowHeight = 0;
		}
		if (applyNewGeometry) {
			item->setGeometry(QRectF(QPointF(next_x + dx, dy), prefSize));
		}
		return top + y + prefSize.height() + bottom;
	}*/

	for (int i = 0; i < _items.size(); ++i) {
		QGraphicsLayoutItem* item = _items.at(i);
		prefSize = item->effectiveSizeHint(Qt::PreferredSize);
		if (i == 0 || i % 6 == 0) {
			maxRowHeight = (maxRowHeight > prefSize.height()) ? maxRowHeight : prefSize.height();
			next_x = x + prefSize.width();
			if (next_x > maxWidth) {
				if (x == 0) {
					prefSize.setWidth(maxWidth);
				}
				else {
					x = 0;
					next_x = prefSize.width();
				}
				y += maxRowHeight + spacing(Qt::Vertical);
				maxRowHeight = 0;
			}
			if (applyNewGeometry) {
				item->setGeometry(QRectF(QPointF(next_x + dx, dy), prefSize));
			}
		}
		else {
			radius = 100;
			/*maxRowHeight = (maxRowHeight > prefSize.height()) ? maxRowHeight : prefSize.height();
			next_x = x + prefSize.width();
			if (next_x > maxWidth) {
				if (x == 0) {
					prefSize.setWidth(maxWidth);
				} else {
					x = 0;
					next_x = prefSize.width();
				}
				y += maxRowHeight + spacing(Qt::Vertical);
				maxRowHeight = 0;
			}
			*/

			x = next_x + spacing(Qt::Horizontal);
			dx = x + radius * qCos(r);
			dy = y + radius * qSin(r);
			if (applyNewGeometry) {
				item->setGeometry(QRectF(QPointF(dx, dy), prefSize));
			}
			r += radius * radian;
		}
	}

	maxRowHeight = (maxRowHeight > prefSize.height()) ? maxRowHeight : prefSize.height();
	return top + y + maxRowHeight + bottom;
}

QSizeF RingLayout::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const {
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

QSizeF RingLayout::minSize(const QSizeF& constraint) const {
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

QSizeF RingLayout::maxSize() const {
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

QSizeF RingLayout::prefSize() const {
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
