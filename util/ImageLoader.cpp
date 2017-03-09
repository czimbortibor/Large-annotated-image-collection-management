#include "ImageLoader.hpp"

ImageLoader::ImageLoader(QString dirName, QList<QString>* imageNames, QList<cv::Mat>& results, const cv::Size& size, QObject* parent) : QObject(parent) {
    _imageNames = imageNames;
    _dirName = dirName;
    _results = &results;
    _size = size;
}

void ImageLoader::run() {
    _running = true;
    int counter = 0;
    /* signal the loaded images every k-th time */
    int k = (_imageNames->length() >> 7) + 1;
    qDebug() << k;
    /* starting index of the results */
    int j = 0;
    for (int i = 0; i < _imageNames->length(); ++i) {
        if (_cancel == 0) {
            QString fullFileName = _dirName + "/" + _imageNames->at(i);
            cv::Mat cvImage = cv::imread(fullFileName.toStdString());
            if (cvImage.data == 0) {
                continue;
            }
            cv::Mat cvResizedImg;
            cv::resize(cvImage, cvResizedImg, _size);
            _results->append(cvResizedImg);
            ++counter;
            if (counter >= k) {
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
    _running = false;
    emit finished();
}
