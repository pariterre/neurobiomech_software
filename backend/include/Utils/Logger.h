#ifndef __STIMWALKER_UTILS_LOGGER_H__
#define __STIMWALKER_UTILS_LOGGER_H__

#include "stimwalkerConfig.h"

#include <fstream>
#include <mutex>

#include "Utils/StimwalkerEvent.h"

namespace STIMWALKER_NAMESPACE::utils {

class Logger {
public:
  // Define the log levels
  enum Level { INFO, WARNING, FATAL };

  // Get the singleton instance of the Logger
  static Logger &getInstance();

  // Log a message at the INFO, WARNING, or FATAL level
  void info(const std::string &message);
  void warning(const std::string &message);
  void fatal(const std::string &message);

  // Set the minimum log level. Messages below this level will not be logged.
  void setLogLevel(Level level);

  // Set the log file to write logs to a file
  void setLogFile(const std::string &filename);

  // Set a event to get a callback when a new log is added
  StimwalkerEvent<std::string> onNewLog;

protected:
  // Private constructor and destructor to prevent direct instantiation
  Logger();
  ~Logger();

  DECLARE_PROTECTED_MEMBER_WITH_SETTER(bool, ShouldPrintToConsole)

  // Log a message if it meets the minimum log level
  void log(const std::string &message, Level level);

  // Function to convert log level to a string label
  std::string getLabel(Level level);

  // Function to get current time as a string
  std::string getCurrentTime();

  // Logger state
  std::mutex mutex_;   // For thread safety
  std::ofstream file_; // Optional file stream for logging to a file
  Level minLogLevel_;  // The minimum log level that will be displayed
};

} // namespace STIMWALKER_NAMESPACE::utils

#endif // __STIMWALKER_UTILS_LOGGER_H__