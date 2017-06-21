#include "LoadingHandler.hpp"

void LoadingHandler::loadImages_mt(const QStringList& imageNames) {
	//_mapper.setWidth(_width).setHeight(_height);
	_mapper = Mapper(_width, _height, *_imageCollection);
	std::function<GraphicsImage(const QString& imageName)> map_functor =
				[&](const QString& imageName) { return _mapper(imageName); };

	auto reduce_lambda = [&](QList<GraphicsImage>& images, const GraphicsImage& image) {
			_reducer(images, image);
    };

	connect(&_reducer, &Reducer::imageReady, [&](const GraphicsImage& image) {
		emit mt_imageReady(image);
	});
	using mt_LoaderPtr = std::unique_ptr<QFuture<QList<GraphicsImage>>>;
	using mt_Loader = QFuture<QList<GraphicsImage>>;
	_loaderMT = mt_LoaderPtr(new mt_Loader(QtConcurrent::mappedReduced<QList<GraphicsImage>>
                                           (imageNames, map_functor, reduce_lambda)));

	using mt_Watcher = QFutureWatcher<QList<GraphicsImage>>;
	_loaderWatcherMT = std::unique_ptr<mt_Watcher>(new mt_Watcher);
	_loaderWatcherMT->setFuture(*_loaderMT.get());
    connect(_loaderWatcherMT.get(), &mt_Watcher::finished, this, &LoadingHandler::onFinishedLoading);
}

QList<GraphicsImage>* LoadingHandler::loadImages_st(QStringList* imageNames) {
	QList<GraphicsImage>* results = new QList<GraphicsImage>();
    cv::Size size(_width, _height);
	_loaderST = std::unique_ptr<ImageLoader>(new ImageLoader(*imageNames, *results, size, *_imageCollection));
    connect(_loaderST.get(), &ImageLoader::resultReady, this, &LoadingHandler::onImageReady);
    connect(_loaderST.get(), &ImageLoader::finished, this, &LoadingHandler::onFinishedLoading);
    QThreadPool::globalInstance()->start(_loaderST.get());
    return results;
}

void LoadingHandler::onImageReady(int index, const QString& url, const QString& originalUrl) {
    emit imageReady(index, url, originalUrl);
}

void LoadingHandler::onFinishedLoading() {
    emit finishedLoading();
}

void LoadingHandler::onCancel() {
    if (_loaderST) {
        if (_loaderST->isRunning()) {
            _loaderST->cancel();
        }
    }
    if (_loaderMT) {
        if (_loaderWatcherMT->isRunning()) {
            _loaderWatcherMT->cancel();
        }
    }
}
