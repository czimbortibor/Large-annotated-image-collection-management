/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "LayoutItem.hpp"

LayoutItem::LayoutItem(QGraphicsItem* parent/* = 0*/, const QImage& image)
    : QGraphicsLayoutItem(), QGraphicsItem(parent) {

	_pix = QPixmap::fromImage(image);
	this->_width = image.width();
	this->_height = image.height();
	QGraphicsLayoutItem::setGraphicsItem(this);

    QGraphicsItem::setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

void LayoutItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
					   QWidget* widget /*= 0*/) {
    Q_UNUSED(widget);
    Q_UNUSED(option);

	painter->drawPixmap(QPointF(), _pix);;
}

QRectF LayoutItem::boundingRect() const {
    return QRectF(QPointF(0,0), geometry().size());
}

void LayoutItem::setGeometry(const QRectF& geom) {
	QGraphicsItem::prepareGeometryChange();
    QGraphicsLayoutItem::setGeometry(geom);
	QGraphicsItem::setPos(geom.topLeft());
}

QSizeF LayoutItem::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const {
    switch (which) {
        case Qt::MinimumSize:
        case Qt::PreferredSize:
            // Do not allow a size smaller than the pixmap with two frames around it
			return _pix.size() + QSize(1, 1);
        case Qt::MaximumSize:
            return QSizeF(1000,1000);
        default:
            break;
    }
    return constraint;
}

void LayoutItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    qDebug() << "cliiiicked!";
    /*setSelected(true);
    QGraphicsItem::mousePressEvent(event);
    */
}

void LayoutItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    qDebug() << "hooover";
    setScale(2.0);
}