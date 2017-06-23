#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <ctime>


class Logger {
public:
	/**
	 * @brief log a simple message to the stdout and a text file
	 * @param message
	 */
	static void log(const std::string& message) {
		std::time_t now = std::time(nullptr);
		std::string timestamp = std::asctime(std::localtime(&now));
		// new line at the end
		timestamp.pop_back();
		std::ofstream logFile;
		logFile.open(file_name, std::ios::out | std::ios::app);
		std::cout << timestamp << ": " << message << std::endl << std::flush;
		logFile << timestamp << ": " << message << std::endl;
		logFile.close();
	}

	static std::string file_name;
};

#endif // LOGGER_HPP
