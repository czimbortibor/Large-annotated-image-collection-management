#include "CBIR.hpp"
#include <QDebug>

CBIR::CBIR() {

}

bool CBIR::MatCompare::operator()(const cv::Mat& mat1, const cv::Mat& mat2) const {
	cv::Mat res;
	cv::compare(mat1, mat2, res, cv::CMP_GE); // mat1 is greater than or equal to mat2
	return cv::countNonZero(res) > 0;
}

bool CBIR::HashCompare::operator()(const cv::Mat& mat1, const cv::Mat& mat2) const {
	double res1 = cv::norm(mat1, cv::NORM_HAMMING2);
	double res2 = cv::norm(mat2, cv::NORM_HAMMING2);
	//qDebug() << "res1:" << res1;
	return res1;
}

std::multimap<double, const cv::Mat>& CBIR::computeHashes(QVector<cv::Mat>& images) {
	std::multimap<double, const cv::Mat>* resMap = new std::multimap<double, const cv::Mat>;
	cv::Mat hashMat;
	std::vector<cv::Mat>* hashes = new std::vector<cv::Mat>;
	//cv::Ptr<cv::img_hash::ImgHashBase> hasher = cv::img_hash::AverageHash::create();

	// compute the hash from each image
	double hashNorm;
	for (auto& image : images) {
		cv::Mat resizedImg;
		cv::resize(image, resizedImg, cv::Size(150, 150));
		//averageHash(resizedImg, hashMat);
		pHash(resizedImg, hashMat);
		hashes->push_back(hashMat);
		// get the norm for every hash
		hashNorm = cv::norm(hashMat, cv::NORM_HAMMING);

		resMap->insert(std::pair<double, const cv::Mat>(hashNorm, resizedImg));
	}

	// compute every hash's average difference from the others
	/*for (uint i = 0; i < hashes->size(); ++i) {
		double sum = 0;
		for (uint j = i+1; j < hashes->size(); ++j) {
			double diff = difference(hashes->at(i), hashes->at(j));
			sum += diff;
		}
		double avgDiff = sum / hashes->size();
		resMap->insert(std::pair<double, const cv::Mat>(avgDiff, images[i]));
			// --- optimize? : http://www.cplusplus.com/reference/map/multimap/insert/
	}*/

	hashes->clear();
	return *resMap;
}

double CBIR::difference(const cv::Mat& mat1, const cv::Mat& mat2) {
	return cv::norm(mat1, mat2, cv::NORM_HAMMING);
}

void CBIR::averageHash(cv::InputArray inputArr, cv::OutputArray outputArr) {
	cv::Mat bitsImg;
	cv::Mat grayImg;
	cv::Mat resizeImg;
	cv::Mat const input = inputArr.getMat();
	CV_Assert(input.type() == CV_8UC4 ||
			  input.type() == CV_8UC3 ||
			  input.type() == CV_8U);

	cv::resize(input, resizeImg, cv::Size(8,8));
	if(input.type() == CV_8UC3)
	{
		cv::cvtColor(resizeImg, grayImg, CV_BGR2GRAY);
	}
	else if(input.type() == CV_8UC4)
	{
		cv::cvtColor(resizeImg, grayImg, CV_BGRA2GRAY);
	}
	else
	{
		grayImg = resizeImg;
	}

	uchar const imgMean = static_cast<uchar>(cvRound(cv::mean(grayImg)[0]));
	cv::compare(grayImg, imgMean, bitsImg, cv::CMP_GT);
	bitsImg /= 255;
	outputArr.create(1, 8, CV_8U);
	cv::Mat hash = outputArr.getMat();
	uchar *hash_ptr = hash.ptr<uchar>(0);
	uchar const *bits_ptr = bitsImg.ptr<uchar>(0);
	std::bitset<8> bits;
	for(size_t i = 0, j = 0; i != bitsImg.total(); ++j)
	{
		for(size_t k = 0; k != 8; ++k)
		{
			//avoid warning C4800, casting do not work
			bits[k] = bits_ptr[i++] != 0;
		}
		hash_ptr[j] = static_cast<uchar>(bits.to_ulong());
	}
}

void CBIR::pHash(cv::InputArray inputArr, cv::OutputArray outputArr) {
	cv::Mat bitsImg;
	cv::Mat dctImg;
	cv::Mat grayFImg;
	cv::Mat grayImg;
	cv::Mat resizeImg;
	cv::Mat topLeftDCT;
	cv::Mat const input = inputArr.getMat();
	CV_Assert(input.type() == CV_8UC4 ||
			  input.type() == CV_8UC3 ||
			  input.type() == CV_8U);

	cv::resize(input, resizeImg, cv::Size(32,32));
	if(input.type() == CV_8UC3)
	{
		cv::cvtColor(resizeImg, grayImg, CV_BGR2GRAY);
	}
	else if(input.type() == CV_8UC4)
	{
		cv::cvtColor(resizeImg, grayImg, CV_BGRA2GRAY);
	}
	else
	{
		grayImg = resizeImg;
	}

	grayImg.convertTo(grayFImg, CV_32F);
	cv::dct(grayFImg, dctImg);
	dctImg(cv::Rect(0, 0, 8, 8)).copyTo(topLeftDCT);
	topLeftDCT.at<float>(0, 0) = 0;
	float const imgMean = static_cast<float>(cv::mean(topLeftDCT)[0]);

	cv::compare(topLeftDCT, imgMean, bitsImg, cv::CMP_GT);
	bitsImg /= 255;
	outputArr.create(1, 8, CV_8U);
	cv::Mat hash = outputArr.getMat();
	uchar *hash_ptr = hash.ptr<uchar>(0);
	uchar const *bits_ptr = bitsImg.ptr<uchar>(0);
	std::bitset<8> bits;
	for(size_t i = 0, j = 0; i != bitsImg.total(); ++j)
	{
		for(size_t k = 0; k != 8; ++k)
		{
			//avoid warning C4800, casting do not work
			bits[k] = bits_ptr[i++] != 0;
		}
		hash_ptr[j] = static_cast<uchar>(bits.to_ulong());
	}
}
