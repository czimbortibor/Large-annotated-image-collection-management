#include "DbContext.hpp"

mongocxx::uri DbContext::loadUri() {
	QFile config_file;
	config_file.setFileName("config.json");
	QJsonDocument dbConfig;
	QJsonObject data;
	if (config_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QString contents = config_file.readAll();
		config_file.close();

		dbConfig = QJsonDocument::fromJson(contents.toUtf8());
		data = dbConfig.object().value("database").toObject();
	}
	else {
		// default database connection URI found in the resources
		config_file.setFileName(":/config/db/db_config.json");
		config_file.open(QIODevice::ReadOnly | QIODevice::Text);
		QString contents = config_file.readAll();
		config_file.close();

		dbConfig = QJsonDocument::fromJson(contents.toUtf8());
		data = dbConfig.object();
	}

	uri = data["URI"].toString().toStdString();
	databaseName = data["db_name"].toString().toStdString();
	feedsNameCollection_name = data["feeds_name_collection"].toString().toStdString();
	feedsCollection_name = data["feeds_collection"].toString().toStdString();
	imageCollection_name = data["image_collection"].toString().toStdString();

	return mongocxx::uri(uri + "/" + databaseName);
}

bool DbContext::init() {
	class noop_logger : public mongocxx::logger {
	   public:
		virtual void operator()(mongocxx::log_level, mongocxx::stdx::string_view,
								mongocxx::stdx::string_view) noexcept {
		}
	};

	auto instance = mongocxx::stdx::make_unique<mongocxx::instance>
			(mongocxx::stdx::make_unique<noop_logger>());

	mongocxx::uri uri = DbContext::loadUri();
	MongoAccess::instance().configure(std::move(instance),
									mongocxx::stdx::make_unique<mongocxx::pool>(std::move(uri)));
}

using bsoncxx::builder::stream::document;
using bsoncxx::document::element;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::finalize;

QJsonArray DbContext::queryAll() {
	auto query = document{} << finalize;
	mongocxx::cursor cursor = feedsCollection.find(query.view());

	QJsonArray results;
	for (const auto& doc : cursor) {
		std::vector<std::string> doc_keys;
		std::transform(std::begin(doc), std::end(doc), std::back_inserter(doc_keys), [](element ele) {
			// key() returns a string_view
			return ele.key().to_string();
		});
		QJsonObject json_obj;
		for (const auto& key : doc_keys) {
			if (doc[key].type() == bsoncxx::type::k_utf8) {
				auto value = doc[key].get_utf8().value.to_string();
				json_obj.insert(QString::fromStdString(key), QString::fromStdString(value));
			}
		results.append(QJsonValue(json_obj));
		}
	}
	return results;
}

QJsonArray DbContext::filterText(const QString& text) {
	auto query = document{} << "$text" << open_document << "$search" << text.toStdString()
							 << close_document << finalize;
	mongocxx::cursor cursor = feedsCollection.find(query.view());

	QJsonArray results;
	for (const auto& doc : cursor) {
		std::vector<std::string> doc_keys;
		std::transform(std::begin(doc), std::end(doc), std::back_inserter(doc_keys), [](element ele) {
			// key() returns a string_view
			return ele.key().to_string();
		});
		QJsonObject json_obj;
		for (const auto& key : doc_keys) {
			if (doc[key].type() == bsoncxx::type::k_utf8) {
				auto value = doc[key].get_utf8().value.to_string();
				json_obj.insert(QString::fromStdString(key), QString::fromStdString(value));
			}
		results.append(QJsonValue(json_obj));
		}
	}
	return results;
}
