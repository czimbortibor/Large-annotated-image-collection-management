#ifndef REDUCER_HPP
#define REDUCER_HPP

#include <QObject>
#include <QList>

#include <opencv2/core.hpp>

#include "../ImageCollection.hpp"


class Reducer : public QObject {
    Q_OBJECT
public:
	Reducer() = default;
    ~Reducer() = default;
	void operator()(QList<cv::Mat>& images, const cv::Mat& image) {
		//images.append(image);
        // TODO: pass URL
		//imageCollection.insert(&image);
        emit imageReady(image, "");
    }

signals:
	void imageReady(const cv::Mat& image, const QString& url);
};

Q_DECLARE_METATYPE(cv::Mat)

#endif // REDUCER_HPP
