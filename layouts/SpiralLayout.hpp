#ifndef SPIRALLAYOUT_HPP
#define SPIRALLAYOUT_HPP

#include <qmath.h>

#include <QWidget>
#include <QDebug>
#include <QPainter>

#include "AbstractGraphicsLayout.hpp"


class SpiralLayout : public AbstractGraphicsLayout {
public:
    explicit SpiralLayout();

    void setSpacing(Qt::Orientations orientation, qreal spacing) Q_DECL_OVERRIDE;
    qreal spacing(Qt::Orientation orientation) const Q_DECL_OVERRIDE { return _spacing[int(orientation) - 1]; }
    void setGeometry(const QRectF& geometry) Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE { return _items.count(); }
    QGraphicsLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE { return _items.at(index); }
    void removeAt(int index) Q_DECL_OVERRIDE { _items.removeAt(index); invalidate(); }

    void addItem(QGraphicsLayoutItem* item) Q_DECL_OVERRIDE { insertItem(-1, item); }
    void clearAll() Q_DECL_OVERRIDE;
    QList<QGraphicsLayoutItem*>& items() Q_DECL_OVERRIDE { return _items; }

    void setTurn(int value) { _a = value; }
    void setDistance(int value) { _b = value; }

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

    qreal _a = 1;
    qreal _b;

};

#endif // SPIRALLAYOUT_HPP
