#ifndef CBIR_HPP
#define CBIR_HPP

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
	explicit CBIR();

	/** compares 2 cv::Mat objects */
	struct MatCompare {
		double operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const;
	};

	struct HashCompare {
		bool operator()(const ulong64& hashA, const ulong64& hashB) const;
	};

	/** using OpenCV's img_hash library */
    std::multimap<const cv::Mat, const cv::Mat, MatCompare>* computeHashes(const QList<cv::Mat>& images,
            cv::Ptr<cv::img_hash::ImgHashBase> hasher);
    //std::multimap<double, const cv::Mat>& computeHashes(QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const;

	/** using the pHash library */
    std::multimap<ulong64, const cv::Mat, HashCompare>& computeHashes_pHash(QList<cv::Mat>& images,
                                                                            const QString& dirname, QList<QString>& imageNames) const;

    /** returns the image's hash value */
    static cv::Mat getHash(const cv::Mat& image);
    static double getHashValue(const cv::Mat& image);
    static double getDistance(const cv::Mat& hashmatA, const cv::Mat& hashmatB);
    static cv::Ptr<cv::img_hash::ImgHashBase> _hasher;

private:

};

#endif // CBIR_HPP
