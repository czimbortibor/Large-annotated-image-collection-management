#include "ImageCollection.hpp"

using ImageMap = std::map<QString, ImageCollection::Collection>;

ImageCollection::ImageCollection() {
    init();
}

void ImageCollection::init() {
    _hashingAlgorithms << "Average hash" << "Perceptual hash" << "Marr Hildreth hash"
    << "Radial Variance hash" << "Block Mean hash" << "Color Moment hash";

    _hashers.emplace("Average hash", cv::img_hash::AverageHash::create());
    _hashers.emplace("Perceptual hash", cv::img_hash::PHash::create());
    _hashers.emplace("Marr Hildreth hash", cv::img_hash::MarrHildrethHash::create());
    _hashers.emplace("Radial Variance hash", cv::img_hash::RadialVarianceHash::create());
    _hashers.emplace("Block Mean hash", cv::img_hash::BlockMeanHash::create());
    _hashers.emplace("Color Moment hash", cv::img_hash::ColorMomentHash::create());

    _collection_map.emplace("Average hash", ImageMap());
    _collection_map.emplace("Perceptual hash", ImageMap());
    _collection_map.emplace("Marr Hildreth hash", ImageMap());
    _collection_map.emplace("Radial Variance hash", ImageMap());
    _collection_map.emplace("Block Mean hash", ImageMap());
    _collection_map.emplace("Color Moment hash", ImageMap());
}

void ImageCollection::insert(cv::Mat* image, QString* url, QString* originalUrl) {
    /** compute the image's hash value with every function,
     * then insert the results into the map */
   for (const auto& hasher : _hashers) {
        /** calculate the hash */
        cv::Mat* hash = new cv::Mat(_cbir.getHash(*image, hasher.second));
        /** insert the results into the hasher's map */
        _collection_map.at(hasher.first).emplace(*url, Collection(image, hash, originalUrl));
    }
}

QList<cv::Mat> ImageCollection::getHashes(const QString& hasherName) const {
    auto imageMap = _collection_map.at(hasherName);
    QList<cv::Mat>* result = new QList<cv::Mat>;
    for (const auto& image_struct : imageMap) {
        result->push_front(image_struct.second.getHash());
    }
    return *result;
}

std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>* ImageCollection::getHashedImages(
        const QString &hasherName) {
    std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>* result = new std::multimap<cv::Mat, cv::Mat, CBIR::MatCompare>;
    auto imageMap = _collection_map.at(hasherName);
    _cbir.setHasher(_hashers.at(hasherName));
    for (const auto& image_struct : imageMap) {
        result->emplace(image_struct.second.getHash(), image_struct.second.getImage());
    }
    return result;
}

QList<cv::Mat>* ImageCollection::getSimilarImages(const QString& url, const QString& hasherName) {
    QList<cv::Mat>* results = new QList<cv::Mat>;
    cv::Mat targetImage = getImage(hasherName, url);
    cv::Ptr<cv::img_hash::ImgHashBase> hasher;
    try {
        hasher = _hashers.at(hasherName);
    }
    catch (std::out_of_range ex) {
        qWarning() << ex.what();
    }

    _cbir.setHasher(hasher);
    cv::Mat targetHash = _cbir.getHash(targetImage, hasher);

    struct CustomCompare {
        CustomCompare(ImageCollection* parent, cv::Mat targetHash) : _parent(parent) {
            _targetHash = targetHash;
        }
        bool operator()(const cv::Mat& hashmatA, const cv::Mat& hashmatB) const {
            double dist1 = _parent->_cbir.getDistance(_targetHash, hashmatA);
            double dist2 = _parent->_cbir.getDistance(_targetHash, hashmatB);
            return dist1 < dist2;
        }

        ImageCollection* _parent;
        cv::Mat _targetHash;
    };

    CustomCompare compareFunctor(this, targetHash);

    std::multimap<cv::Mat, cv::Mat, CustomCompare> resMap(compareFunctor);
    std::map<const QString, Collection> imageMap;
    try {
        imageMap = _collection_map.at(hasherName);
    }
    catch (std::out_of_range ex) {
            qWarning() << ex.what();
    }

    for (const auto& image_struct : imageMap) {
        resMap.emplace(image_struct.second.getHash(), image_struct.second.getImage());
    }

    for (const auto& images : resMap) {
        results->push_back(images.second);
    }
    return results;
}