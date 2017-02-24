#ifndef VIEW_HPP
#define VIEW_HPP

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QLibrary>
#include <QOpenGLWidget>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "util/LayoutItem.hpp"
#include "layouts/FlowLayout.hpp"
#include "layouts/RingLayout.hpp"


class GraphicsView : public QGraphicsView {
public:
	explicit GraphicsView();
	void init();
	void setLayout(AbstractGraphicsLayout* layout) { _layout = layout; _layout->invalidate(); }
	void setMinSceneSize(const QSizeF value) { _layoutWidget->setMinimumSize(value); _layout->invalidate();}
	void addItem(QGraphicsLayoutItem* item) { _layout->addItem(item); }
	void clear() { _layout->clearAll(); }

	// expose the scene to get it's signals
	QGraphicsScene* scene() { return _scene; }

	template<typename L> void displayImages(const QVector<cv::Mat>& images, const L& layout) const;
	/** opencv img_hash & pHash display */
	template<typename T, typename L> void displayImages(const T& images, const L& layout) const;

private:
	QImage Mat2QImage(const cv::Mat& cvImage) const;

	QOpenGLWidget* _glWidget;
	QGraphicsScene* _scene;
	QGraphicsWidget* _layoutWidget;
	AbstractGraphicsLayout* _layout;
};

#endif // VIEW_HPP
