#ifndef METADATAPARSER_HPP
#define METADATAPARSER_HPP

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>

#include <QDebug>

#include "ImageCollection.hpp"


class MetadataParser {
public:
	MetadataParser() {}
	~MetadataParser() {}

	static QList<cv::Mat>& getImages(const QJsonArray& metadata,
											const ImageCollection& imageCollection);
};

#endif // METADATAPARSER_HPP
