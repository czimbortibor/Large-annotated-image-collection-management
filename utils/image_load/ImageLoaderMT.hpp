#ifndef IMAGELOADERMT_HPP
#define IMAGELOADERMT_HPP


#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>


#include "utils/image_load/Mapper.hpp"
#include "utils/image_load/Reducer.hpp"

/**
 * @brief handles image loading in a multi-threaded asynchronous way
 */
class ImageLoaderMT : public QObject {
	Q_OBJECT
public:
	ImageLoaderMT(const QStringList& imageNames, int width, int height,
				  ImageCollection& imageCollection) {
		_imageNames = imageNames;
		_width = width;
		_height = height;
		_imageCollection = &imageCollection;
	}

	~ImageLoaderMT() = default;

	void run();


	QFutureWatcher<QList<GraphicsImage>> loaderWatcher;

private:
	QFuture<QList<GraphicsImage>> _loader;

	Mapper _mapper;
	Reducer _reducer;
	int _width;
	int _height;
	QStringList _imageNames;
	ImageCollection* _imageCollection;

signals:
	void imageReady(const GraphicsImage& image);
};

#endif // IMAGELOADERMT_HPP
