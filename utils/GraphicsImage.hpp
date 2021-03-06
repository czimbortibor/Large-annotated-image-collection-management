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

#ifndef GRAPHICSIMAGE_HPP
#define GRAPHICSIMAGE_HPP

#include <QtGui>
#include <QGraphicsLayout>
#include <QGraphicsLayoutItem>
#include <QGraphicsObject>
#include <QDebug>

#include <memory>
#include <opencv2/core.hpp>

#include "graphics/SelectEffect.hpp"


class GraphicsImage : public QGraphicsObject, public QGraphicsLayoutItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_INTERFACES(QGraphicsLayoutItem)
public:
	GraphicsImage() = default;
	GraphicsImage(const QImage& image, const QString& url, const QString& originalUrl);
	GraphicsImage(const QImage& image);
	GraphicsImage(const GraphicsImage& other);
	GraphicsImage& operator=(const GraphicsImage& other);
	/*~GraphicsImage() {
		setGraphicsEffect(nullptr);
		QGraphicsLayoutItem::setGraphicsItem(0);
	}*/
	~GraphicsImage() = default;

    // Inherited from QGraphicsLayoutItem
	void setGeometry(const QRectF& geom);
	QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

    // Inherited from QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

    qreal getWidth() const { return _width; }
    qreal getHeight() const { return _height; }
	QPixmap getPixmap() const { return _pix; }
    QString getUrl() const { return _url; }
    QString getOriginalUrl() const { return _originalUrl; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

private:
	QPixmap _pix;
    QString _url;
    QString _originalUrl;
	qreal _width;
	qreal _height;

signals:
    void clicked(const QString& url);
    void doubleClick(const QString& url);
	void hoverEnter(const QString& url, GraphicsImage*);
    void hoverLeave();
};

#endif //GRAPHICSIMAGE_HPP
