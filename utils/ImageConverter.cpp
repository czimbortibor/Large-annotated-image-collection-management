#include "ImageConverter.hpp"

QImage ImageConverter::Mat2QImage(const cv::Mat& cvimage) {
    switch (cvimage.type()) {
         // 8-bit, 4 channel
         case CV_8UC4: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_ARGB32);
            return qimage;
         }

         // 8-bit, 3 channel
         case CV_8UC3: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_RGB888);
            return qimage.rgbSwapped();
         }

         // 8-bit, 1 channel
         case CV_8UC1: {
            QImage qimage(cvimage.data, cvimage.cols, cvimage.rows,
                          static_cast<int>(cvimage.step),
                          QImage::Format_Grayscale8);
            return qimage;
         }

         default: {
			Logger::log("cv::Mat type not handled:" + std::to_string(cvimage.type()));
            return QImage();
         }
      }
}

cv::Mat ImageConverter::QImage2Mat(const QImage& qimage) {
    switch (qimage.format()) {
         // 8-bit, 4 channel
         case QImage::Format_ARGB32:
         case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat cvimage(qimage.height(), qimage.width(), CV_8UC4,
                          const_cast<uchar*>(qimage.bits()),
                          static_cast<size_t>(qimage.bytesPerLine()));
            return cvimage;
         }

         // 8-bit, 3 channel
         case QImage::Format_RGB32:
         case QImage::Format_RGB888: {
            QImage swapped = qimage;
            if (qimage.format() == QImage::Format_RGB32) {
               swapped = swapped.convertToFormat(QImage::Format_RGB888);
            }
            swapped = swapped.rgbSwapped();
            return cv::Mat(swapped.height(), swapped.width(), CV_8UC3,
                            const_cast<uchar*>(swapped.bits()),
                            static_cast<size_t>(swapped.bytesPerLine())).clone();
         }

         // 8-bit, 1 channel
         case QImage::Format_Indexed8: {
            cv::Mat cvimage(qimage.height(), qimage.width(), CV_8UC1,
                          const_cast<uchar*>(qimage.bits()),
                          static_cast<size_t>(qimage.bytesPerLine()));
            return cvimage;
         }

         default: {
			Logger::log("QImage format not handled:" + std::to_string(qimage.format()));
            return cv::Mat();
         }
    }
}
