#ifndef METADATA_HPP
#define METADATA_HPP

#include <string>
#include <chrono>
#include <bsoncxx/types.hpp>


class Metadata {
public:
	Metadata() = default;
	~Metadata() = default;

	std::string link;
	std::string image_url;
	std::string rss;
	std::string author;
	std::string title;
	std::string summary;
	std::string published; //{std::chrono::system_clock::now()};
	std::string image_path;
};

#endif // METADATA_HPP
