#ifndef MAPPER_HPP
#define MAPPER_HPP

#include <QString>
#include <QDir>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


class Mapper {
public:
    Mapper() = default;
    Mapper(const QString& path, const int& width, const int& height) {
        _width = width; _height = height;
    }
    ~Mapper() = default;

    using result_type = cv::Mat;

    cv::Mat operator()(const QString& imageName) {
        QString fileName = _path + QDir::separator() + imageName;
        cv::Mat cvImage = cv::imread(fileName.toStdString());
        if (cvImage.data == 0) {
            cv::Mat empty;
            return empty;
        }
        cv::Mat cvResizedImg;
        cv::resize(cvImage, cvResizedImg, cv::Size(_width, _height));
        return cvResizedImg;
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
    QString _path;
    int _width;
    int _height;
};

#endif // MAPPER_HPP
