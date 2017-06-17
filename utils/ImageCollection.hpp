#ifndef IMAGECOLLECTION_HPP
#define IMAGECOLLECTION_HPP

#include <memory>
#include <map>
#include <functional>

#include <QMultiMap>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/img_hash.hpp>

#include "CBIR.hpp"
#include "Logger.hpp"


class ImageCollection {
public:
    ImageCollection();
    ~ImageCollection() = default;

    /**
     * @brief define the hashing algorithm types -> collection_map key
     */
    void init();

    QStringList getHashingAlgorithms() const { return _hashingAlgorithms; }

    void insert(cv::Mat* image, QString* url, QString* originalUrl);

    cv::Mat getHashValue(const QString& hasherName, QString& url) {
        return _collection_map.at(hasherName).at(url).getHash();
    }

    cv::Ptr<cv::img_hash::ImgHashBase> getHasher(const QString& hasherName) const {
        return _hashers.at(hasherName);
    }

    cv::Mat getImage(const QString& hasherName, const QString& url) {
        return _collection_map.at(hasherName).at(url).getImage();
    }

    /**
     * @brief returns all the results from the hasherName hash algorithm
     * @param hasherName hash algorithm
     * @return list containing the hash values
     */
    QList<cv::Mat> getHashes(const QString& hasherName) const;

    /**
     * @brief getHashedImages returns all of the images in order of their hash values
     * @param hasherName hash algorithm
	 * @return the hashed images
     */
	QList<cv::Mat>* getHashedImages(const QString& hasherName);

    /**
     * @brief getSimilarImages returns the images in order of their similarity to the selected image
     * @param url the selected image's url
     * @param hasherName
     * @return a list containing the images in a specific order
     */
    QList<cv::Mat>* getSimilarImages(const QString& url, const QString& hasherName);

	QList<cv::Mat>* getImagesByUrl(const QStringList& imgUrls) const;

    struct Collection {
    public:
        Collection(cv::Mat* image, cv::Mat* hash, QString* originalUrl) {
            _image = image;
            _hash = hash;
            _originalUrl = originalUrl;
        }
        const cv::Mat& getImage() const { return *_image; }
        const cv::Mat& getHash() const { return *_hash; }
        const QString& getOriginalUrl() const { return *_originalUrl; }

    private:
        const cv::Mat* _image;
        const cv::Mat* _hash;
        const QString* _originalUrl;
    };

private:
    CBIR _cbir;

    using ImageMap = std::map<const QString, Collection>;
    std::map<QString, ImageMap> _collection_map;

    /**
     * @brief hashing algorithms' name
     */
    QStringList _hashingAlgorithms;

    /**
     * @brief hashing funtions
     */
    std::map<QString, cv::Ptr<cv::img_hash::ImgHashBase>> _hashers;
};

#endif // IMAGECOLLECTION_HPP
