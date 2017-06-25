#include "CBIR.hpp"

cv::Ptr<cv::img_hash::ImgHashBase> CBIR::static_hasher;

CBIR::CBIR() {
    // disable opencl acceleration, may(or may not) boost up speed of img_hash
    cv::ocl::setUseOpenCL(false);
}

double CBIR::MatCompare::operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const {
    return static_hasher->compare(hashmatA, hashmatB);
}

using ImageMap = std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>;

ImageMap* CBIR::computeHashes(const QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) {
    ImageMap* resMap = new ImageMap;
    static_hasher = hasher;
	// compute the hash from each image
    QList<cv::Mat>::const_iterator iter;
    for (iter = images.begin(); iter != images.end(); ++iter) {
        cv::Mat hashMat;
        static_hasher->compute(*iter, hashMat);
        resMap->emplace(hashMat, *iter);
	}
    return resMap;
}

cv::Mat CBIR::getHash(const cv::Mat& image, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const {
    cv::Mat hashMat;
    hasher->compute(image, hashMat);
    return hashMat;
}

double CBIR::getHashValue(const cv::Mat& image) const {
    cv::Mat hashMat;
    static_hasher->compute(image, hashMat);
    return cv::norm(hashMat, cv::NORM_HAMMING);
}

double CBIR::getDistance(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const {
    return static_hasher->compare(hashmatA, hashmatB);
}
