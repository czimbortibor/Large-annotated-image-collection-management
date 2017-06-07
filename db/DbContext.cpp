#include "DbContext.hpp"


DbContext::DbContext() {
	QFile config_file(":/config/db/db_config.json");
	config_file.open(QIODevice::ReadOnly | QIODevice::Text);
	QString contents = config_file.readAll();
	config_file.close();

	QJsonDocument dbConfig = QJsonDocument::fromJson(contents.toUtf8());
	QJsonObject data = dbConfig.object();

	_URI = data["URI"].toString().toStdString();
	_databaseName = data["db_name"].toString().toStdString();
	_feedsNameCollection_name = data["feeds_name_collection"].toString().toStdString();
	_feedsCollection_name = data["feeds_collection"].toString().toStdString();
	_imageCollection_name = data["image_collection"].toString().toStdString();
}

bool DbContext::init() {
	// specify the host
	mongocxx::uri uri(_URI);
	try {
		// connect to the server
		_client = mongocxx::client(uri);

		// access the database
		_db = mongocxx::database(_client[_databaseName]);

		// get the collection
		_feedsNameCollection = mongocxx::collection(_db[_feedsNameCollection_name]);
		// check if the collection exists
		try {
			_feedsNameCollection.name();
		}
		catch (const mongocxx::operation_exception& ex) {
            std::cerr << "database collection error:" << ex.what() << std::flush;
			return false;
		}
	}
	catch (const mongocxx::exception& ex) {
        std::cerr << "database connection failed: " << ex.what() << std::flush;
		return false;
	}
	return true;
}

QStringList* DbContext::test() {
    auto document = bsoncxx::builder::stream::document{} << "created_at" << "673849510750257152";
//	mongocxx::cursor res = _collection.find(document << bsoncxx::builder::stream::finalize);
//	for (auto&& doc : res) {
//        std::cout << bsoncxx::to_json(doc) << std::flush;
 //   }

	int feed_count = _feedsNameCollection.count({});

   // using bsoncxx::builder::stream::finalize;
   // using bsoncxx::types::value;
   // std::string regexpDate = "/.*" + testStr + ".*/i";
	// with picture
    //std::string regexpPic = "entities.media.0.display_url": { $exists: true }
    /*auto regex = bsoncxx::types::b_regex(regexpDate, "");
    bsoncxx::builder::stream::document filter;
    filter << "created_at" << value{regex};
    std::cout << bsoncxx::to_json(filter) << "\n" << std::flush;
    mongocxx::cursor cursor = _collection.find(filter << finalize);*/

    QStringList* results = new QStringList;
    *results << "695609799455584258_1.jpg"
            << "695609932029280256_1.jpg";
    return results;
}
