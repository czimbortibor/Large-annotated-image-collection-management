#include "CBIR.hpp"

CBIR::CBIR() {
	// disable opencl acceleration, may(or may not) boost up speed of img_hash
	cv::ocl::setUseOpenCL(false);
}

double CBIR::MatCompare::operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const {
	double res = cv::norm(hashmatA, hashmatB, cv::NORM_HAMMING);
	qDebug() << res;
	return res;
}

bool CBIR::HashCompare::operator()(const ulong64& hashA, const ulong64& hashB) const {
	return ph_hamming_distance(hashA, hashB);
}

//std::multimap<const cv::Mat, const cv::Mat, CBIR::MatCompare>& CBIR::computeHashes(QVector<cv::Mat>& images) const {
std::multimap<double, const cv::Mat>& CBIR::computeHashes(QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const {
	//std::multimap<const cv::Mat, const cv::Mat, MatCompare>* resMap = new std::multimap<const cv::Mat, const cv::Mat, MatCompare>;
	std::multimap<double, const cv::Mat>* resMap = new std::multimap<double, const cv::Mat>;

	// compute the hash from each image
	for (const auto& image : images) {
		double hashNorm;
		cv::Mat hashMat;

		hasher->compute(image, hashMat);
		// get the norm for every hash
		hashNorm = cv::norm(hashMat, cv::NORM_HAMMING);
		//resMap->insert(std::pair<const cv::Mat, const cv::Mat>(hashMat, image));
		resMap->insert(std::pair<double, const cv::Mat>(hashNorm, image));
	}

	return *resMap;
}

std::multimap<ulong64, const cv::Mat, CBIR::HashCompare>& CBIR::computeHashes_pHash(QVector<cv::Mat>& images, const QString& dirname, QList<QString>& imageNames) const {
	std::multimap<ulong64, const cv::Mat, HashCompare>* resMap = new std::multimap<ulong64, const cv::Mat, HashCompare>;

	for (int i = 0; i < images.size(); ++i) {
		std::string filename(QString(dirname + '/' + imageNames[i]).toStdString());
		// compute hash
		ulong64 hash;
		ph_dct_imagehash(filename.c_str(), hash);
		resMap->insert(std::pair<ulong64, const cv::Mat>(hash, images[i]));
	}

	return *resMap;
}
