#include "MongoAccess.hpp"

MongoAccess::MongoAccess(const std::string& hostName, const std::string& databaseName, const std::string& collectionName) {
	_hostName = hostName;
	_databaseName = databaseName;
	_collectionName = collectionName;
}

bool MongoAccess::init() {
	// specify the host
	mongocxx::uri uri(_hostName);
	try {
		// connect to the server
		_client = mongocxx::client(uri);

		// check if the connection was succesful, else fails with a mongocxx::exception
		mongocxx::cursor databases = _client.list_databases();
		databases.begin();

		// access the database
		_db = mongocxx::database(_client[_databaseName]);

		// get the collection
		_collection = mongocxx::collection(_db[_collectionName]);
		// check if the collection exists
		try {
			_collection.name();
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

QStringList* MongoAccess::test(const std::string& date1, const std::string& date2) {
    auto document = bsoncxx::builder::stream::document{} << "created_at" << "673849510750257152";
	mongocxx::cursor res = _collection.find(document << bsoncxx::builder::stream::finalize);
	for (auto&& doc : res) {
        std::cout << bsoncxx::to_json(doc) << std::flush;
    }

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
