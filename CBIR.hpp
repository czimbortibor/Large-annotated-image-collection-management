#ifndef CBIR_HPP
#define CBIR_HPP

#include <QVector>

#include <bitset>
#include <map>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/img_hash.hpp>

class CBIR {
public:
	CBIR();

	/** compares 2 cv::Mat objects */
	struct MatCompare {
		bool operator()(const cv::Mat& mat1, const cv::Mat& mat2) const;
	};

	struct HashCompare {
		bool operator()(const cv::Mat& mat1, const cv::Mat& mat2) const;
	};

	std::multimap<double, const cv::Mat>& computeHashes(QVector<cv::Mat>& images);

private:
	void averageHash(cv::InputArray inputArr, cv::OutputArray outputArr);
	void pHash(cv::InputArray inputArr, cv::OutputArray outputArr);
	double difference(const cv::Mat& mat1, const cv::Mat& mat2);

};

#endif // CBIR_HPP
