#ifndef GRAPHICSVIEW_HPP
#define GRAPHICSVIEW_HPP

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#include <map>
#include <memory>

#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QLibrary>
#include <QOpenGLWidget>
#include <QLabel>
#include <QGraphicsProxyWidget>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "utils/LayoutItem.hpp"
#include "utils/FlowLayoutFactory.hpp"
#include "utils/PetalLayoutFactory.hpp"
#include "utils/SpiralLayoutFactory.hpp"
#include "utils/Logger.hpp"


class GraphicsView : public QGraphicsView {
public:
	explicit GraphicsView(QWidget* parent = 0);
	void init();
	/** makes a new layout from the available factories and sets it onto the scene */
	void setLayout(const QString& value);
	void setMinSceneSize(const QSizeF value) { _layoutWidget->setMinimumSize(value); _layout->invalidate();}
	void addItem(const QGraphicsLayoutItem* item);
	int itemCount() const { return _layout->count(); }
    void clear() { _layout->clearAll(); }
    void addPopupImage(QLabel* label, LayoutItem* item);

	/*void resizeEvent(QResizeEvent* event) {
		fitInView(_scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
	}*/

    void wheelEvent(QWheelEvent* event);

	/** expose a reference to the scene to get it's signals; reference, so the ownership won't move */
	QGraphicsScene& scene() { return *_scene; }

    //template<typename L> void displayImages(const QVector<cv::Mat>& images, const L& layout) const;
	/** opencv img_hash & pHash display */
    //template<typename T, typename L> void displayImages(const T& images, const L& layout) const;


	void setNrOfPetals(int value);
	void setRadius(double value);

    void setSpiralDistance(int value);
    void setSpiralTurn(int value);

protected:
    /*void mousePressEvent(QMouseEvent* event) {
        QGraphicsView::mousePressEvent(event);
    }*/

private:
	QImage Mat2QImage(const cv::Mat& cvImage) const;

	QOpenGLWidget* _glWidget;
	QGraphicsScene* _scene;
	QGraphicsWidget* _layoutWidget;
	AbstractGraphicsLayout* _layout;

    std::unique_ptr<QGraphicsProxyWidget> _proxyLabel;

	std::unique_ptr<std::map<std::string, AbstractLayoutFactory*>> _layouts;

signals:
    void imageClick(QGraphicsItem* image);

public slots:
	void onAddItem(const QGraphicsLayoutItem* item) {
		addItem(item);
	}

private slots:
    void onSceneRectChanged(const QRectF& rect);
    void onImageClicked(QGraphicsItem* image) { emit imageClick(image); }
    void onRemovePopup();
};

#endif // GRAPHICSVIEW_HPP
