#include "CBIR.hpp"

cv::Ptr<cv::img_hash::ImgHashBase> CBIR::static_hasher;

CBIR::CBIR() {
	// disable opencl acceleration, may(or may not) boost up speed of img_hash
    cv::ocl::setUseOpenCL(false);
}

double CBIR::MatCompare::operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const {
    return static_hasher->compare(hashmatA, hashmatB);
}

bool CBIR::HashCompare::operator()(const ulong64& hashA, const ulong64& hashB) const {
	return ph_hamming_distance(hashA, hashB);
}

using ImageMap = std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>;

ImageMap* CBIR::computeHashes(const QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) {
//std::multimap<double, const cv::Mat>& CBIR::computeHashes(QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const {
    //std::multimap<double, const cv::Mat>* resMap = new std::multimap<double, const cv::Mat>;
    ImageMap* resMap = new ImageMap;
    static_hasher = hasher;
	// compute the hash from each image
    QList<cv::Mat>::const_iterator iter;
    for (iter = images.begin(); iter != images.end(); ++iter) {
        //double hashNorm;
        cv::Mat hashMat;

        static_hasher->compute(*iter, hashMat);

		// get the norm for every hash
        //hashNorm = cv::norm(hashMat, cv::NORM_HAMMING);
        //resMap->emplace(hashNorm, *iter);

        resMap->emplace(hashMat, *iter);

        //images.erase(iter);
	}
    return resMap;
}

std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>& CBIR::computeHashes_pHash(QList<cv::Mat>& images, const QString& dirname, QList<QString>& imageNames) const {
    std::multimap<ulong64, const cv::Mat, HashCompare>* resMap = new std::multimap<ulong64, const cv::Mat, HashCompare>;

	for (int i = 0; i < images.size(); ++i) {
		std::string filename(QString(dirname + '/' + imageNames[i]).toStdString());
		// compute hash
		ulong64 hash;
		ph_dct_imagehash(filename.c_str(), hash);
        resMap->emplace(hash, images[i]);
	}

	return *resMap;
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
