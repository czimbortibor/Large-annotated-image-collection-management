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

#include <pHash.h>

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

	struct HashCompare {
		bool operator()(const ulong64& hashA, const ulong64& hashB) const;
	};

	/** using OpenCV's img_hash library */
    std::map<cv::Mat, cv::Mat, MatCompare>* computeHashes(const QList<cv::Mat>& images,
            cv::Ptr<cv::img_hash::ImgHashBase> hasher);
    //std::multimap<double, const cv::Mat>& computeHashes(QList<cv::Mat>& images, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const;
    std::set<cv::Mat, MatCompare>* getHashes(const QList<cv::Mat>& images,
                                             cv::Ptr<cv::img_hash::ImgHashBase> hasher);
	/** using the pHash library */
    std::multimap<ulong64, const cv::Mat, HashCompare>& computeHashes_pHash(QList<cv::Mat>& images,
                                                                            const QString& dirname, QList<QString>& imageNames) const;

    void setHasher(cv::Ptr<cv::img_hash::ImgHashBase> hasher) { static_hasher = hasher; }
    /** returns the image's hash value */
    cv::Mat getHash(const cv::Mat& image, cv::Ptr<cv::img_hash::ImgHashBase> hasher) const;
    double getHashValue(const cv::Mat& image) const;
    double getDistance(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const;
    static cv::Ptr<cv::img_hash::ImgHashBase> static_hasher;

private:
    cv::Ptr<cv::img_hash::ImgHashBase> _hasher;
};

#endif // CBIR_HPP
