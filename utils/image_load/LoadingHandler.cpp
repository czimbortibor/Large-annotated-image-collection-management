#include "LoadingHandler.hpp"

void LoadingHandler::loadImages_mt(const QString& path, const QStringList& imageNames) {
    _mapper.setPath(path).setWidth(_width).setHeight(_height);
    std::function<cv::Mat(const QString& imageName)> map_functor =
                [this](const QString& imageName) { return _mapper(imageName); };

    auto reduce_lambda = [this] (QList<cv::Mat>& images, const cv::Mat& image) {
            _reducer(images, image);
    };

    // TODO: multi-thread image load PROPERLY!
    //connect(&_reducer, &Reducer::imageReady, this, &LoadingHandler::onImageReady);
    using mt_LoaderPtr = std::unique_ptr<QFuture<QList<cv::Mat>>>;
    using mt_Loader = QFuture<QList<cv::Mat>>;
    _loaderMT = mt_LoaderPtr(new mt_Loader(QtConcurrent::mappedReduced<QList<cv::Mat>>
                                           (imageNames, map_functor, reduce_lambda)));

    using mt_Watcher = QFutureWatcher<QList<cv::Mat>>;
    _loaderWatcherMT->setFuture(*_loaderMT.get());
    connect(_loaderWatcherMT.get(), &mt_Watcher::finished, this, &LoadingHandler::onFinishedLoading);
}

QList<cv::Mat>* LoadingHandler::loadImages_st(const QString& path, QStringList* imageNames) {
    QList<cv::Mat>* results = new QList<cv::Mat>;
    cv::Size size(_width, _height);
    _loaderST = std::unique_ptr<ImageLoader>(new ImageLoader(path, *imageNames,
                                                                *results, size, *_imageCollection));
    connect(_loaderST.get(), &ImageLoader::resultReady, this, &LoadingHandler::onImageReady);
    connect(_loaderST.get(), &ImageLoader::finished, this, &LoadingHandler::onFinishedLoading);
    QThreadPool::globalInstance()->start(_loaderST.get());
    return results;
}

void LoadingHandler::onImageReady(int index, const QString& url) {
    emit imageReady(index, url);
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
