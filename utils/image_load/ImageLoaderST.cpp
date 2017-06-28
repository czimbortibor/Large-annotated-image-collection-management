#include "ImageLoaderST.hpp"

ImageLoaderST::ImageLoaderST(QStringList& imageNames,
						 QList<GraphicsImage>& results,
						 const cv::Size& size,
						 ImageCollection& imageCollection, QObject* parent) : QObject(parent) {
    _imageNames = &imageNames;
    _results = &results;
    _size = size;
    _imageCollection = &imageCollection;
}

void ImageLoaderST::run() {
    _running.testAndSetOrdered(0, 1);
    for (int i = 0; i < _imageNames->length(); ++i) {
        if (static_cast<int>(_running)) {
			//QString* fullFileName = new QString(_dirName + "/" + _imageNames->at(i));
			QString* fileName = new QString(_imageNames->at(i));
			if (fileName->endsWith(".gif")) {
				continue;
			}
			//QString* originalFileName = new QString(_originalDirPath + "/" + _imageNames->at(i));
			cv::Mat cvImage;
			cvImage = cv::imread(fileName->toStdString());
            if (cvImage.data == 0) {
                continue;
            }
			cv::Mat* cvResizedImg = new cv::Mat();
            cv::resize(cvImage, *cvResizedImg, _size);
			GraphicsImage* image = new GraphicsImage(ImageConverter::Mat2QImage(*cvResizedImg),
											   *fileName, *fileName);
			//image->mat.reset(cvResizedImg);
			_results->append(*image);
			_imageCollection->insert(cvResizedImg, fileName, fileName);
			emit resultReady(_index);
			++_index;
            }
        }
    _running.testAndSetOrdered(1, 0);
    emit finished();
}
