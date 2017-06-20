#include "ImageLoader.hpp"

ImageLoader::ImageLoader(const QString dirName, QStringList& imageNames,
						 QList<GraphicsImage>& results,
                         const cv::Size& size,
                         ImageCollection& imageCollection,
                         const QString& originalDirPath, QObject* parent) : QObject(parent) {
    _imageNames = &imageNames;
    _dirName = dirName;
    _originalDirPath = originalDirPath;
    _results = &results;
    _size = size;
    _imageCollection = &imageCollection;
}

void ImageLoader::run() {
    _running.testAndSetOrdered(0, 1);
    for (int i = 0; i < _imageNames->length(); ++i) {
        if (static_cast<int>(_running)) {
            QString* fullFileName = new QString(_dirName + "/" + _imageNames->at(i));
			if (fullFileName->endsWith(".gif")) {
				continue;
			}
            QString* originalFileName = new QString(_originalDirPath + "/" + _imageNames->at(i));
			cv::Mat cvImage;
            cvImage = cv::imread(fullFileName->toStdString());
            if (cvImage.data == 0) {
                continue;
            }
			cv::Mat* cvResizedImg = new cv::Mat();
            cv::resize(cvImage, *cvResizedImg, _size);
			GraphicsImage* image = new GraphicsImage(ImageConverter::Mat2QImage(*cvResizedImg),
											   *fullFileName, *originalFileName);
			image->mat.reset(cvResizedImg);
			_results->append(*image);
			_imageCollection->insert(cvResizedImg, fullFileName, originalFileName);
			emit resultReady(_index, *fullFileName, *originalFileName);
			++_index;
            }
        }
    _running.testAndSetOrdered(1, 0);
    emit finished();
}
