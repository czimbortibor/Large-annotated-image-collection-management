/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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

#ifndef FLOWLAYOUT_HPP
#define FLOWLAYOUT_HPP

#include <qmath.h>
#include <QWidget>

#include "AbstractGraphicsLayout.hpp"


class FlowLayout : public AbstractGraphicsLayout {
public:
    FlowLayout();

	void setSpacing(Qt::Orientations o, qreal spacing) Q_DECL_OVERRIDE;
	qreal spacing(Qt::Orientation o) const Q_DECL_OVERRIDE;
	void setGeometry(const QRectF& geom) Q_DECL_OVERRIDE;
	int count() const Q_DECL_OVERRIDE;
	QGraphicsLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE;
    void removeAt(int index) Q_DECL_OVERRIDE;

	void addItem(QGraphicsLayoutItem* item) Q_DECL_OVERRIDE { insertItem(-1, item); }
	void clearAll() Q_DECL_OVERRIDE;

protected:
	void insertItem(int index, QGraphicsLayoutItem* item) Q_DECL_OVERRIDE;
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const Q_DECL_OVERRIDE;

private:
	qreal doLayout(const QRectF& geom, bool applyNewGeometry) const Q_DECL_OVERRIDE;
	QSizeF minSize(const QSizeF& constraint) const Q_DECL_OVERRIDE;
	QSizeF prefSize() const Q_DECL_OVERRIDE;
	QSizeF maxSize() const Q_DECL_OVERRIDE;

	QList<QGraphicsLayoutItem*> m_items;
    qreal m_spacing[2];
};

#endif // FLOWLAYOUT_HPP
