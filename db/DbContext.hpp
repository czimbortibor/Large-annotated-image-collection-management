#ifndef DB_HPP
#define DB_HPP

#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

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

class DbContext {
public:
	explicit DbContext();
	bool init();

	QStringList* test();

private:
	std::string _URI;
	std::string _databaseName;
	std::string _feedsNameCollection_name;
	std::string _feedsCollection_name;
	std::string _imageCollection_name;
	/** connection to a running MongoDB instance */
	mongocxx::client _client;
	/** instance to a database */
	mongocxx::database _db;
	/** instance to a collection in the database */
	mongocxx::collection _feedsNameCollection;
	mongocxx::collection _feedsCollection;
	mongocxx::collection _imageCollection;
};

#endif // DB_HPP
