#include "ImageLoader.hpp"

ImageLoader::ImageLoader(QString dirName, QList<QString>* imageNames,
                         QList<cv::Mat>& results,
                         const cv::Size& size, int notifyRate, QObject* parent) : QObject(parent) {
    _imageNames = imageNames;
    _dirName = dirName;
    _results = &results;
    _size = size;
    _notifyRate = notifyRate;
}

void ImageLoader::run() {
    _running.testAndSetOrdered(0, 1);
    int counter = 0;
    // starting index of the results
    int j = 0;
    for (int i = 0; i < _imageNames->length(); ++i) {
        if (static_cast<int>(_running)) {
            QString fullFileName = _dirName + "/" + _imageNames->at(i);
            cv::Mat cvImage;
            cvImage = cv::imread(fullFileName.toStdString());
            if (cvImage.data == 0) {
                continue;
            }
            cv::Mat cvResizedImg;
            cv::resize(cvImage, cvResizedImg, _size);
            _results->append(cvResizedImg);
            ++counter;
            if (counter >= _notifyRate) {
                emit resultsReadyAt(j, i);
                counter = 0;
                j = i;
            }
        }
        else {
            return;
        }
    }
    // if loaded images are left
    if (counter) {
        emit resultsReadyAt(j, j+counter);
    }
    _running.testAndSetOrdered(1, 0);
    emit finished();
}
