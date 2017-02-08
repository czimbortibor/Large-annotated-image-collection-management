#ifndef CBIR_HPP
#define CBIR_HPP

#include <QVector>
#include <QList>
#include <QDebug>

#include <bitset>
#include <map>
#include <vector>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/img_hash.hpp>

#include <pHash.h>

class CBIR {
public:
	CBIR();

	/** compares 2 cv::Mat objects */
	struct MatCompare {
		double operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const;
	};

	struct HashCompare {
		bool operator()(const ulong64& hashA, const ulong64& hashB) const;
	};

	/** opencv img_hash */
	//std::multimap<const cv::Mat, const cv::Mat, MatCompare>& computeHashes(QVector<cv::Mat>& images) const;
	std::multimap<double, const cv::Mat>& computeHashes(QVector<cv::Mat>& images) const;

	/** pHash */
	std::multimap<ulong64, const cv::Mat, HashCompare>& computeHashes_pHash(QVector<cv::Mat>& images, const QString& dirname, QList<QString>& imageNames) const;

private:
	void averageHash(cv::InputArray inputArr, cv::OutputArray outputArr) const;
	void pHash(cv::InputArray inputArr, cv::OutputArray outputArr) const;
};

#endif // CBIR_HPP
