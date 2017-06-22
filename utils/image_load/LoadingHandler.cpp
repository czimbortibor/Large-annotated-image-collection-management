#include "LoadingHandler.hpp"

void LoadingHandler::loadImages_mt(QStringList* imageNames) {
	_loaderMT = std::unique_ptr<ImageLoaderMT>
			(new ImageLoaderMT(*imageNames, _width, _height, *_imageCollection));
	connect(_loaderMT.get(), &ImageLoaderMT::imageReady, [&](const GraphicsImage& result) {
		emit imageReady_mt(result);
	});
	using LoaderWatcher = QFutureWatcher<QList<GraphicsImage>>;
	connect(&_loaderMT->loaderWatcher, &LoaderWatcher::finished, this, &LoadingHandler::onFinishedLoading);

	_loaderMT->run();
}

QList<GraphicsImage>* LoadingHandler::loadImages_st(QStringList* imageNames) {
	QList<GraphicsImage>* results = new QList<GraphicsImage>();
    cv::Size size(_width, _height);
	_loaderST = std::unique_ptr<ImageLoaderST>(new ImageLoaderST(*imageNames, *results, size, *_imageCollection));
	connect(_loaderST.get(), &ImageLoaderST::resultReady, [&](int index) {
		emit imageReady_st(index);
	});
	connect(_loaderST.get(), &ImageLoaderST::finished, this, &LoadingHandler::onFinishedLoading);

    QThreadPool::globalInstance()->start(_loaderST.get());
    return results;
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
		if (_loaderMT->loaderWatcher.isRunning()) {
			_loaderMT->loaderWatcher.cancel();
        }
    }
}
