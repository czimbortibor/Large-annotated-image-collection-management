#ifndef REDUCER_HPP
#define REDUCER_HPP

#include <QObject>
#include <QList>

#include <opencv2/core.hpp>


class Reducer : public QObject {
    Q_OBJECT
public:
    Reducer() = default;
    ~Reducer() = default;
    void operator()(QList<cv::Mat>& images, const cv::Mat& image) {
        //images.push_front(image);
        emit imageReady(image);
    }
signals:
    void imageReady(const cv::Mat& image);
};

Q_DECLARE_METATYPE(cv::Mat)

#endif // REDUCER_HPP
