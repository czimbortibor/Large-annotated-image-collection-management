#ifndef METADATA_HPP
#define METADATA_HPP

#include <string>
#include <chrono>
#include <bsoncxx/types.hpp>

#include <map>
#include <set>


class Metadata {
public:
	Metadata() = default;
	~Metadata() = default;

	std::string& operator[](const std::string& key) {
		return _data[key];
	}

	const std::string& operator[](const std::string& key) const {
		return _data.at(key);
	}

	std::set<std::string> keys() const {
		return _keys;
	}

	std::string link;
	std::string image_url;
	std::string rss;
	std::string author;
	std::string title;
	std::string summary;
	std::string published; //{std::chrono::system_clock::now()};
	std::string image_path;

private:
	std::set<std::string> _keys = {"link", "image_url", "rss", "author", "title", "summary",
								  "published", "image_path"};
	std::map<std::string, std::string> _data;
};

#endif // METADATA_HPP
