#ifndef CBIR_HPP
#define CBIR_HPP

#include <QList>
#include <QDebug>

#include <bitset>
#include <map>
#include <set>
#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/img_hash.hpp>

class CBIR {
public:
    explicit CBIR();

    struct MatKey : cv::Mat {
        double operator<(const MatKey& other) {
            return CBIR::static_hasher->compare(*this, other);
        }
    };

	/** compares 2 cv::Mat objects */
	struct MatCompare {
        double operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const;
	};

	/** using OpenCV's img_hash library */
    std::multimap<cv::Mat, cv::Mat, MatCompare>* computeHashes(const QList<cv::Mat>& images,
            cv::Ptr<cv::img_hash::ImgHashBase> hasher);
    //std::multimap<double, const cv::Mat>& computeHashes(QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const;

    void setHasher(cv::Ptr<cv::img_hash::ImgHashBase> hasher) { static_hasher = hasher; }
    /** returns the image's hash value */
	cv::Mat getHash(const cv::Mat& image, const cv::Ptr<cv::img_hash::ImgHashBase>& hasher) const;
    double getHashValue(const cv::Mat& image) const;
    double getDistance(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const;
    static cv::Ptr<cv::img_hash::ImgHashBase> static_hasher;

private:
    cv::Ptr<cv::img_hash::ImgHashBase> _hasher;
};

#endif // CBIR_HPP
