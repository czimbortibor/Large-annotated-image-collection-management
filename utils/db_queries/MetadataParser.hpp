#ifndef METADATAPARSER_HPP
#define METADATAPARSER_HPP

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>

#include <QDebug>

#include "../ImageCollection.hpp"
#include "Metadata.hpp"


class MetadataParser {
public:
	MetadataParser() {}
	~MetadataParser() {}

	static QList<Metadata>& getMetadata(const QJsonArray& metadata);

	/**
	 * @brief selects the images from the metadata
	 * @param metadata
	 * @param imageCollection
	 * @return
	 */
	static QList<GraphicsImage>& getImages(const QList<Metadata>& metadata,
											const ImageCollection& imageCollection);

};

#endif // METADATAPARSER_HPP
