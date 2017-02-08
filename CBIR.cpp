#include "CBIR.hpp"

CBIR::CBIR() {

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
std::multimap<double, const cv::Mat>& CBIR::computeHashes(QVector<cv::Mat>& images) const {
	//std::multimap<const cv::Mat, const cv::Mat, MatCompare>* resMap = new std::multimap<const cv::Mat, const cv::Mat, MatCompare>;
	std::multimap<double, const cv::Mat>* resMap = new std::multimap<double, const cv::Mat>;

	//cv::Ptr<cv::img_hash::ImgHashBase> hasher = cv::img_hash::AverageHash::create();

	// compute the hash from each image
	for (const auto& image : images) {
		double hashNorm;
		cv::Mat hashMat;
		//averageHash(resizedImg, hashMat);
		pHash(image, hashMat);
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

void CBIR::averageHash(cv::InputArray inputArr, cv::OutputArray outputArr) const {
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

void CBIR::pHash(cv::InputArray inputArr, cv::OutputArray outputArr) const {
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
