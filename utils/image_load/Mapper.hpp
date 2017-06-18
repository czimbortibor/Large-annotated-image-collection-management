#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <QString>
#include <QDir>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "../ImageCollection.hpp"


class Mapper {
public:
    Mapper() = default;
	Mapper(const QString& path, const int& width, const int& height, ImageCollection& imageCollection) {
		_path = path;
		_width = width;
		_height = height;
		_imageCollection = &imageCollection;
    }
    ~Mapper() = default;

	using result_type = LayoutItem;

	LayoutItem operator()(const QString& imageName) {
		QString* fileName = new QString(_path + QDir::separator() + imageName);
		cv::Mat cvImage = cv::imread(fileName->toStdString());
        if (cvImage.data == 0) {
            cv::Mat empty;
			return LayoutItem(ImageConverter::Mat2QImage(empty), "", "");
        }
		cv::Mat* cvResizedImg = new cv::Mat();
		cv::resize(cvImage, *cvResizedImg, cv::Size(_width, _height));
		LayoutItem* image = new LayoutItem(ImageConverter::Mat2QImage(*cvResizedImg), *fileName, *fileName);
		_imageCollection->insert(cvResizedImg, fileName, fileName);
		return *image;
    }

    Mapper& setPath(const QString& path) {
        _path = path;
        return *this;
    }

    Mapper& setWidth(const int& width) {
        _width = width;
        return *this;
    }

    Mapper& setHeight(const int& height) {
        _height = height;
        return *this;
    }

private:
	ImageCollection* _imageCollection;
    QString _path;
    int _width;
    int _height;
};

#endif // MAPPER_HPP
