#ifndef DB_HPP
#define DB_HPP

#include <QString>

#include <string>
#include <iostream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/options/client.hpp>
#include <mongocxx/exception/operation_exception.hpp>

class MongoAccess {
public:
	explicit MongoAccess(const std::string& hostName, const std::string& databaseName, const std::string& collectionName);
	bool init();

    void test(const std::string& testStr);

private:
	std::string _hostName;
	std::string _databaseName;
	std::string _collectionName;
	/** connection to a running MongoDB instance */
	mongocxx::client _client;
	/** instance to a database */
	mongocxx::database _db;
	/** instance to a collection in the database */
	mongocxx::collection _collection;
};

#endif // DB_HPP
