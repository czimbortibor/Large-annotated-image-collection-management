#include "DbContext.hpp"

mongocxx::uri DbContext::loadUri() {
	QFile config_file(":/config/db/db_config.json");
	config_file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString contents = config_file.readAll();
	config_file.close();

	QJsonDocument dbConfig = QJsonDocument::fromJson(contents.toUtf8());
	QJsonObject data = dbConfig.object();

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
