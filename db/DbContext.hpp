#ifndef DBCONTEXT_HPP
#define DBCONTEXT_HPP

#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDebug>

#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>

#include <bsoncxx/array/view.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>
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
	void init();

	QJsonArray queryAll();
	QJsonArray queryText(const QString& text);
	QJsonArray queryImagePath(const QString& image_path);
	QJsonArray queryImagePaths(const QStringList& image_paths);

	static std::string bdate_to_string(const bsoncxx::document::element& bdate) {
		std::chrono::system_clock::time_point tp = bdate.get_date();
		std::time_t time = std::chrono::system_clock::to_time_t(tp);
		std::string time_str = std::ctime(&time);
		return time_str;
	}


	/* instances to collections in the database */
	mongocxx::collection feedsNameCollection;
	mongocxx::collection feedsCollection;
	mongocxx::collection imageCollection;

	std::string uri;
	std::string databaseName;
	std::string feedsNameCollection_name;
	std::string feedsCollection_name;
	std::string imageCollection_name;

	std::vector<std::string> _keys = {"link", "image_url", "rss", "author", "title", "summary",
								  "published", "image_path"};

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
		~MongoAccess() = default;

		std::unique_ptr<mongocxx::instance> _instance = nullptr;
		std::unique_ptr<mongocxx::pool> _pool = nullptr;
	}; // </MongoAccess>

}; // </DbContext>

#endif // DBCONTEXT_HPP
