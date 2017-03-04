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
			std::cerr << "database collection error:" << ex.what() << "\n";
			return false;
		}
	}
	catch (const mongocxx::exception& ex) {
		std::cerr << "database connection failed: " << ex.what() << "\n";
		return false;
	}
	return true;
}

void MongoAccess::test() {
    /*auto document = bsoncxx::builder::stream::document{} << "id_str" << "673849483520815108";
	mongocxx::cursor res = _collection.find(document << bsoncxx::builder::stream::finalize);
	for (auto&& doc : res) {
		std::cout << bsoncxx::to_json(doc) << "\n";
    }*/
   /* bsoncxx::types::b_regex regex('', '');
    auto document = bsoncxx::builder::stream::document{} << "display_url";
    mongocxx::cursor res = _collection.find(document << bsoncxx::builder::stream::finalize);
    for (auto&& doc : res) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }*/
}
