#include "MongoAccess.hpp"

MongoAccess::MongoAccess(const std::string& hostName, const std::string& databaseName, const std::string& collectionName) {
	_hostName = hostName;
	_databaseName = databaseName;
	_collectionName = collectionName;
}

bool MongoAccess::init() {
	try {
		// specify the host
		mongocxx::uri uri(_hostName);
		// connect to the server
		_client = mongocxx::client(uri);
		// access the database
		_db = mongocxx::database(_client[_databaseName]);
		// get the collection
		_collection = mongocxx::collection(_db[_collectionName]);
		std::cout << "database connection is succesful" << "\n";
		return true;
	}
	catch(const std::exception& ex) {
		std::cerr << "database connection failed: " << ex.what() << "\n";
		return false;
	}
}

void MongoAccess::test() {
	auto document = bsoncxx::builder::stream::document{} << "id_str" << "673849483520815108";
	mongocxx::cursor res = _collection.find(document << bsoncxx::builder::stream::finalize);
	for (auto&& doc : res) {
		std::cout << bsoncxx::to_json(doc) << "\n";
	}
}
