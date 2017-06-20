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

#include "GraphicsImage.hpp"

GraphicsImage::GraphicsImage(const QImage& image, const QString& url, const QString& originalUrl): QGraphicsObject(), QGraphicsLayoutItem() {
    _pix = QPixmap::fromImage(image);
    _width = image.width();
    _height = image.height();
	QGraphicsLayoutItem::setGraphicsItem(this);
    _url = url;
    _originalUrl = originalUrl;
	mat = std::make_shared<cv::Mat>();
    QGraphicsItem::setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

GraphicsImage::GraphicsImage(const QImage& image) : QGraphicsObject(), QGraphicsLayoutItem() {
    _pix = QPixmap::fromImage(image);
    _width = image.width();
    _height = image.height();
    QGraphicsLayoutItem::setGraphicsItem(this);

    QGraphicsItem::setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
}

GraphicsImage& GraphicsImage::operator=(const GraphicsImage& other) {
    _pix = other.getPixmap();
    _width = other.getWidth();
    _height = other.getHeight();
    _url = other.getUrl();
    _originalUrl = other.getOriginalUrl();
	mat = other.mat;
    QGraphicsLayoutItem::setGraphicsItem(this);
    QGraphicsItem::setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsFocusable);
    return *this;
}

void GraphicsImage::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
					   QWidget* widget /*= 0*/) {
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->drawPixmap(QPointF(), _pix);;
}

QRectF GraphicsImage::boundingRect() const {
	return QRectF(QPointF(0, 0), geometry().size());
}

void GraphicsImage::setGeometry(const QRectF& geom) {
	QGraphicsItem::prepareGeometryChange();
    QGraphicsLayoutItem::setGeometry(geom);
	QGraphicsItem::setPos(geom.topLeft());
}

QSizeF GraphicsImage::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const {
    switch (which) {
        case Qt::MinimumSize:
        case Qt::PreferredSize:
            // Do not allow a size smaller than the pixmap with two frames around it
            return _pix.size() + QSize(1, 1);
        case Qt::MaximumSize:
			return QSizeF(1000, 1000);
        default:
            break;
    }
    return constraint;
}

void GraphicsImage::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(_url);
	if (isSelected()) {
		setSelected(false);
		setGraphicsEffect(nullptr);
	}
	else {
		setSelected(true);
		SelectEffect* effect = new SelectEffect(1.2);
		setGraphicsEffect(effect);
	}
}

void GraphicsImage::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    emit doubleClick(_url);
}

void GraphicsImage::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
	Qt::KeyboardModifiers pressedKeys = QGuiApplication::keyboardModifiers();
	if (pressedKeys == Qt::ShiftModifier) {
		emit hoverEnter(_originalUrl, this);
	}
}

void GraphicsImage::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    emit hoverLeave();
}
