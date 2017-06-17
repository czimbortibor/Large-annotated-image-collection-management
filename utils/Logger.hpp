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
		std::ofstream logFile;
		logFile.open(file_name, std::ios::out | std::ios::app);
		std::cout << timestamp << ": " << message << std::endl << std::flush;
		logFile << timestamp << ": " << message << std::endl;
		logFile.close();
	}

	/**
	 * @brief log the elapsed time and a description to this into the stdout and a text file
	 * @param description
	 * @param elapsed_time
	 */
	static void log_elapsed_time(const std::string& description, double elapsed_time) {
		std::time_t now = std::time(nullptr);
		std::string timestamp = std::asctime(std::localtime(&now));
		std::ofstream logFile;
		logFile.open(file_name, std::ios::out | std::ios::app);
		std::cout << timestamp << ": "
				<< description << std::to_string(elapsed_time) << " seconds" << std::flush;
		logFile << timestamp << ": "
				<< description << std::to_string(elapsed_time) << " seconds" << std::endl;
		logFile.close();
	}

	static std::string file_name;
};

#endif // LOGGER_HPP
