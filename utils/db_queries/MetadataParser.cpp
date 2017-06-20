#include "MetadataParser.hpp"

QList<Metadata>& MetadataParser::getMetadata(const QJsonArray& metadata) {
	QList<Metadata>* results = new QList<Metadata>();
	for (const auto& json : metadata) {
		QJsonObject obj = json.toObject();

		Metadata data;
		data["link"] = obj["link"].toString().toStdString();
		data["image_url"] = obj["image_url"].toString().toStdString();
		data["rss"] = obj["rss"].toString().toStdString();
		data["author"] = obj["author"].toString().toStdString();
		data["title"] = obj["title"].toString().toStdString();
		data["summary"] = obj["summary"].toString().toStdString();
		data["published"] = obj["published"].toString().toStdString();
		data["image_path"]= obj["image_path"].toString().toStdString();

		results->append(data);
	}
	return *results;
}

Metadata& MetadataParser::getMetadata_url(const QString& url) {

}

QList<GraphicsImage>& MetadataParser::getImages(const QList<Metadata>& metadata,
												 const ImageCollection& imageCollection) {
	QStringList imgPaths;
	for (const auto& data : metadata) {
		imgPaths.append(QString::fromStdString(data.image_path));
	}

	QList<GraphicsImage>* results = imageCollection.getImagesByUrl(imgPaths);
	return *results;
}
