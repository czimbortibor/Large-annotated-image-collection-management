#include "ImageLoaderMT.hpp"

void ImageLoaderMT::run() {
	_mapper = Mapper(_width, _height, *_imageCollection);

	std::function<GraphicsImage(const QString& imageName)> map_functor =
				[&](const QString& imageName) { return _mapper(imageName); };

	auto reduce_lambda = [&](QList<GraphicsImage>& images, const GraphicsImage& image) {
			_reducer(images, image);
	};

	connect(&_reducer, &Reducer::imageReady, [&](const GraphicsImage& image) {
		emit imageReady(image);
	});

	_loader = QtConcurrent::mappedReduced<QList<GraphicsImage>>
										   (_imageNames, map_functor, reduce_lambda);

	loaderWatcher.setFuture(_loader);
}
