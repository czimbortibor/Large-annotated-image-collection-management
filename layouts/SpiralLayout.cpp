#include "SpiralLayout.hpp"

SpiralLayout::SpiralLayout() {
    _spacing[0] = 1;
    _spacing[1] = 1;

    QSizePolicy sizePol = sizePolicy();
    // should the widget's preferred height depend on its width
    sizePol.setHeightForWidth(true);
    setSizePolicy(sizePol);
}

void SpiralLayout::insertItem(int index, QGraphicsLayoutItem* item) {
    QGraphicsLayout::addChildLayoutItem(item);
    //item->setParentLayoutItem(this);
    if (index > _items.count()) {
        index = _items.count();
    }
    _items.insert(index, item);
    invalidate();
}

void SpiralLayout::clearAll() {
    // calls delete on every element
    qDeleteAll(_items.begin(), _items.end());
    // removes the elements from the container
    _items.clear();
    invalidate();
}

void SpiralLayout::setSpacing(Qt::Orientations orientation, qreal spacing) {
    if (orientation & Qt::Horizontal) {
        _spacing[0] = spacing;
    }
    if (orientation & Qt::Vertical) {
        _spacing[1] = spacing;
    }
}

void SpiralLayout::setGeometry(const QRectF& geometry) {
    QGraphicsLayout::setGeometry(geometry);
    doLayout(geometry, true);
}

qreal SpiralLayout::doLayout(const QRectF& geometry, bool applyNewGeometry) const {
    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    const qreal maxWidth = geometry.width() - left - right;
    const qreal maxHeight = geometry.height() - top - bottom;

    // scene's center
    qreal x0 = maxWidth / 2;
    qreal y0 = maxHeight / 2;
    qreal maxRowHeight = 0;
    QSizeF prefSize;

    /** Archimedean spiral : r = a + b(angle)
     * a - will turn the spiral
     * b - controls the distance
    */

    qreal angle = 0;
    qreal increment = 2 * M_PI / _items.size();
    qreal x, y;
    for (int i = 0; i < _items.size(); ++i) {
        QGraphicsLayoutItem* item = _items.at(i);
        prefSize = item->effectiveSizeHint(Qt::PreferredSize);

        angle = 0.1 * i + increment;
        x = x0 + (_a + _b * angle) * std::cos(angle);
        y = y0 + (_a + _b * angle) * std::sin(angle);

        qreal next_x;
        next_x = x + prefSize.width() + spacing(Qt::Vertical);

        if (applyNewGeometry) {
            item->setGeometry(QRectF(QPointF(next_x, y), prefSize));
        }

    }
    return top + y + maxRowHeight + bottom;
}

QSizeF SpiralLayout::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const {
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

QSizeF SpiralLayout::minSize(const QSizeF& constraint) const {
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

QSizeF SpiralLayout::maxSize() const {
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

QSizeF SpiralLayout::prefSize() const {
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
