#include "ImageLoader.hpp"

ImageLoader::ImageLoader(const QString dirName, QStringList& imageNames,
                         QList<cv::Mat>& results,
                         const cv::Size& size,
                         ImageCollection& imageCollection, QObject* parent) : QObject(parent) {
    _imageNames = &imageNames;
    _dirName = dirName;
    _results = &results;
    _size = size;
    _imageCollection = &imageCollection;
}

void ImageLoader::run() {
    _running.testAndSetOrdered(0, 1);
    for (int i = 0; i < _imageNames->length(); ++i) {
        if (static_cast<int>(_running)) {
            QString* fullFileName = new QString(_dirName + "/" + _imageNames->at(i));
            cv::Mat cvImage;
            cvImage = cv::imread(fullFileName->toStdString());
            if (cvImage.data == 0) {
                continue;
            }
            cv::Mat* cvResizedImg = new cv::Mat;
            cv::resize(cvImage, *cvResizedImg, _size);
            _results->append(*cvResizedImg);
            _imageCollection->insert(cvResizedImg, fullFileName);
            emit resultReady(i, *fullFileName);
            }
        }
    _running.testAndSetOrdered(1, 0);
    emit finished();
}
