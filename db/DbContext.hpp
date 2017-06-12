#ifndef DBCONTEXT_HPP
#define DBCONTEXT_HPP

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
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/query_exception.hpp>


class DbContext {
public:
	mongocxx::uri loadUri();
	bool init();

	QStringList& filterTitle(const std::string& title);
	QStringList& filterTexts(const QMap<QString, QString>& textfields);

	/* instances to collections in the database */
	mongocxx::collection feedsNameCollection;
	mongocxx::collection feedsCollection;
	mongocxx::collection imageCollection;

	std::string uri;
	std::string databaseName;
	std::string feedsNameCollection_name;
	std::string feedsCollection_name;
	std::string imageCollection_name;

	class MongoAccess {
	public:
		static MongoAccess& instance() {
			static MongoAccess instance;
			return instance;
		}

		void configure(std::unique_ptr<mongocxx::instance> instance,
					   std::unique_ptr<mongocxx::pool> pool) {
			_instance = std::move(instance);
			_pool = std::move(pool);
		}

		using connection = mongocxx::pool::entry;

		connection get_connection() {
			return _pool->acquire();
		}

		mongocxx::stdx::optional<connection> try_get_connection() {
			return _pool->try_acquire();
		}

	private:
		MongoAccess() = default;

		std::unique_ptr<mongocxx::instance> _instance = nullptr;
		std::unique_ptr<mongocxx::pool> _pool = nullptr;
	}; // </MongoAccess>

}; // </DbContext>

#endif // DBCONTEXT_HPP
