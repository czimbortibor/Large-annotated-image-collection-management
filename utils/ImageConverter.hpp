#ifndef IMAGECONVERTER_HPP
#define IMAGECONVERTER_HPP

#include <QImage>
#include <QDebug>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "Logger.hpp"


class ImageConverter {
public:
    ImageConverter() = default;
    static QImage Mat2QImage(const cv::Mat &cvImage);
    static cv::Mat QImage2Mat(const QImage& image);
};

#endif // IMAGECONVERTER_HPP
