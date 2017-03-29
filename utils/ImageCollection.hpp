#ifndef IMAGECOLLECTION_HPP
#define IMAGECOLLECTION_HPP

#include <memory>
#include <map>

#include <QMultiMap>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/img_hash.hpp>

#include "CBIR.hpp"


class ImageCollection {
public:
    ImageCollection();
    ~ImageCollection() = default;

    /**
     * @brief define the hashing algorithm types -> collection_map key
     */
    void init();

    QStringList getHashingAlgorithms() const { return _hashingAlgorithms; }

    void insert(cv::Mat* image, QString* url);

    cv::Mat getHashValue(const QString& hasherName, QString& url) {
        return _collection_map.at(hasherName).at(url)->getHash();
    }

    cv::Ptr<cv::img_hash::ImgHashBase> getHasher(const QString& hasherName) const {
        return _hashers.at(hasherName);
    }

    cv::Mat getImage(const QString& hasherName, QString& url) {
        return _collection_map.at(hasherName).at(url)->getImage();
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
     * @return a map containing the images as values and their hash as the key
     */
    std::map<cv::Mat, cv::Mat, CBIR::MatCompare> getHashedImages(const QString& hasherName);


    struct Collection {
    public:
        Collection(cv::Mat* image, cv::Mat* hash) {
            _image = *image;
            _hash = *hash;
        }
        cv::Mat getImage() const { return _image; }
        cv::Mat getHash() const { return _hash; }
    private:
        cv::Mat _image;
        cv::Mat _hash;
    };

private:
    CBIR _cbir;

    using ImageMap = std::map<QString, Collection*>;
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
