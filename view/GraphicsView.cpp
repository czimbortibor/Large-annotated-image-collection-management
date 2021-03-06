#include "GraphicsView.hpp"

GraphicsView::GraphicsView(QWidget* parent) : QGraphicsView(parent) {
	_layouts = std::unique_ptr<std::map<std::string, AbstractLayoutFactory*>>(new std::map<std::string, AbstractLayoutFactory*>);
	init();
}

void GraphicsView::init() {
	// try to load the OpenGL library
	QLibrary lib("GL");
	// if OpenGL is present on the machine then enable hardware accelerated graphics on the scene
	if (lib.load()) {
		Logger::log("OpenGL loaded");
		_glWidget = new QOpenGLWidget;
		setViewport(_glWidget);
	}

	_scene = new QGraphicsScene;
	setScene(_scene);
    connect(_scene, &QGraphicsScene::sceneRectChanged, this, &GraphicsView::onSceneRectChanged);

	// align from the top-left corner rather than from the center
	//setAlignment(Qt::AlignLeft | Qt::AlignTop);

	_layoutWidget = new QGraphicsWidget;

	// initialize the layout factories
    _layouts->emplace("grid", new FlowLayoutFactory);
    _layouts->emplace("petal", new PetalLayoutFactory);
    _layouts->emplace("spiral", new SpiralLayoutFactory);

	// make the default FlowLayout and set it onto the scene
    _layout = _layouts->at("grid")->makeLayout();
	_layoutWidget->setLayout(_layout);
	_scene->addItem(_layoutWidget);
}

void GraphicsView::onSceneRectChanged(const QRectF& rect) {
	Q_UNUSED(rect)
    this->setMinSceneSize(this->size());
}

void GraphicsView::addItem(const QGraphicsLayoutItem* item) {
	//QGraphicsLayoutItem* tmp = item;
	_layout->addItem(const_cast<QGraphicsLayoutItem*>(item));
	const GraphicsImage* layoutItem = static_cast<const GraphicsImage*>(item);
	connect(layoutItem, &GraphicsImage::hoverLeave, this, &GraphicsView::onRemovePopup, Qt::DirectConnection);
}

void GraphicsView::setLayout(const QString& value) {
	AbstractGraphicsLayout* tmp = _layouts->at(value.toStdString())->makeLayout();

	// TODO: overload operator=

	for (const auto& item : _layout->items()) {
        GraphicsImage* layoutItem = static_cast<GraphicsImage*>(item);
        connect(layoutItem, &GraphicsImage::hoverLeave, this, &GraphicsView::onRemovePopup, Qt::DirectConnection);
		tmp->addItem(item);
	}
	_layout->clearAll();
	_layout = tmp;
	_layoutWidget->setLayout(_layout);

    this->setMinSceneSize(this->size());
}

void GraphicsView::setNrOfPetals(int value) {
	const auto layoutPtr = static_cast<PetalLayout*>(_layout);
	if (layoutPtr != nullptr) {
		layoutPtr->setNrOfPetals(value);
	}
}

void GraphicsView::setRadius(double value) {
	const auto layoutPtr = static_cast<PetalLayout*>(_layout);
	if (layoutPtr != nullptr) {
		layoutPtr->setRadius(value);
	}
}

void GraphicsView::setSpiralDistance(int value) {
    const auto layoutPtr = static_cast<SpiralLayout*>(_layout);
    if (layoutPtr != nullptr) {
        layoutPtr->setDistance(value);
    }
}

void GraphicsView::setSpiralTurn(int value) {
    const auto layoutPtr = static_cast<SpiralLayout*>(_layout);
    if (layoutPtr != nullptr) {
        layoutPtr->setTurn(value);
    }
}

void GraphicsView::addPopupImage(QLabel* label, GraphicsImage* item) {
    _proxyLabel.reset(_scene->addWidget(label));
    _proxyLabel->setAcceptHoverEvents(false);
    QPointF pos = item->pos();
    QPointF newPos;
    QRectF labelSize = _proxyLabel->geometry();
    if (pos.y() - labelSize.height() > 0) {
        newPos = QPointF(pos.x(), pos.y() - labelSize.height());
    }
    else {
        if (pos.y() + labelSize.height() < _scene->height()) {
            newPos = QPointF(pos.x(), labelSize.height());
        }
    }
    _proxyLabel->setPos(newPos);
    //_proxyLabel->setWindowFlags(Qt::Popup);
}

void GraphicsView::onRemovePopup() {
	if (_proxyLabel) {
		_scene->removeItem(_proxyLabel.get());
		_proxyLabel.release();
	}
}

void GraphicsView::resizeEvent(QResizeEvent* event) {
	//fitInView(_scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
}

void GraphicsView::wheelEvent(QWheelEvent* event) {
	Qt::KeyboardModifiers pressedKeys = QGuiApplication::keyboardModifiers();
	if (pressedKeys == Qt::ControlModifier) {
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		double scaleFactor = 1.5;
		/* wheel goes forward -> zoom in */
		if (event->delta() > 0) {
			scale(scaleFactor, scaleFactor);
		}
		/* zoom out */
		else {
			scale(1/scaleFactor, 1/scaleFactor);
		}
	}
	else {
		QGraphicsView::wheelEvent(event);
	}
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event) {
	Q_UNUSED(event)

}

QList<GraphicsImage>& GraphicsView::getSelectedImages() {
	QList<GraphicsImage>* results = new QList<GraphicsImage>();
	for (const auto& item : _scene->selectedItems()) {
		results->append(*(static_cast<GraphicsImage*>(item)));
	}
	return *results;
}


