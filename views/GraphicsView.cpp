#include "GraphicsView.hpp"

GraphicsView::GraphicsView() {
	init();
}

void GraphicsView::init() {
	_scene = new QGraphicsScene;
	setScene(_scene);
	// align from the top-left corner rather than from the center
	//setAlignment(Qt::AlignLeft | Qt::AlignTop);

	_layoutWidget = new QGraphicsWidget;
	_layout = new FlowLayout;
	_layoutWidget->setLayout(_layout);
	_scene->addItem(_layoutWidget);
}

template<typename L>
void GraphicsView::displayImages(const QVector<cv::Mat>& images, const L& layout) const {
	for (const auto& image : images) {
		QImage res = Mat2QImage(image);
		LayoutItem* item = new LayoutItem(NULL, res);
		layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

template<typename T, typename L>
void GraphicsView::displayImages(const T& images, const L& layout) const {
	for (const auto& entry : images) {
		QImage image = Mat2QImage(entry.second);
		LayoutItem* item = new LayoutItem(NULL, image);
		layout->addItem(static_cast<QGraphicsLayoutItem*>(item));
	}
}

QImage GraphicsView::Mat2QImage(const cv::Mat& image) const {
	//return QImage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888).rgbSwapped();

	cv::Mat temp;
	// makes a copy in RGB format
	cvtColor(image, temp, CV_BGR2RGB);
	QImage dest((const uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	// enforce deep copy
	dest.bits();
	return dest;
}
