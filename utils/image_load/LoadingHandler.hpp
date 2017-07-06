#ifndef LOADINGHANDLER_HPP
#define LOADINGHANDLER_HPP

#include <memory>

#include <QObject>

#include "utils/image_load/ImageLoaderST.hpp"
#include "utils/image_load/ImageLoaderMT.hpp"
#include "utils/ImageConverter.hpp"


class LoadingHandler : public QObject {
    Q_OBJECT
public:
	LoadingHandler(ImageCollection& imageCollection) {
		_imageCollection = &imageCollection;
    }

	~LoadingHandler();

    /**
     * @brief loadImages_mt loads the images located at the chosen path
	 * using multiple threads. The loaded images will be accessible via signals
     * @param imageNames the file names
     */
	void loadImages_mt(QStringList* imageNames);

    /**
     * @brief loadImages_st loads the images located at the chosen path
     * using a single thread
     * @param imageNames the file names
     * @return the loaded images
     */
	QList<GraphicsImage>* loadImages_st(QStringList* imageNames);

    cv::Mat loadImage(const QString& fileName) const;

    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }

private:
	ImageCollection* _imageCollection;
	std::unique_ptr<QList<GraphicsImage>> _images;
    /**
     * @brief the given width of the image to be loaded
     */
    int _width = 100;
    /**
     * @brief the given height of the image to be loaded
     */
    int _height = 100;

	struct LoaderMT_deleter {
		void operator()(ImageLoaderMT* ptr) {
			delete ptr;
		}
	};

	// ----------- multi-threaded image load -----------
	std::unique_ptr<ImageLoaderMT, LoaderMT_deleter> _loaderMT;

	struct LoaderST_deleter {
		void operator()(ImageLoaderST* ptr) {
			delete ptr;
		}
	};

	// ---------- single-threaded image load -----------
	std::unique_ptr<ImageLoaderST, LoaderST_deleter> _loaderST;

signals:
	void imageReady_st(int index);
	void imageReady_mt(const GraphicsImage& image);
    void finishedLoading();

public slots:
	void onFinishedLoading();
    void onCancel();
};

#endif // LOADINGHANDLER_HPP
