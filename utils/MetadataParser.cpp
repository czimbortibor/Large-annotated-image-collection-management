#include "MetadataParser.hpp"

QList<cv::Mat>& MetadataParser::getImages(const QJsonArray& metadata,
												 const ImageCollection& imageCollection) {
	QStringList* imgPaths = new QStringList();
	for (const auto& json : metadata) {
		QJsonObject obj = json.toObject();
		if (obj.contains("image_path")) {
			QString imgPath = obj["image_path"].toString();
			imgPaths->append(imgPath);
		}
	}

	QList<cv::Mat>* results = imageCollection.getImagesByUrl(*imgPaths);
	return *results;
}