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
  enum Level { DEBUG, INFO, WARNING, FATAL };

  // Get the singleton instance of the Logger
  static Logger &getInstance();

  // Log a message at the DEBUG, INFO, WARNING, or FATAL level
  void debug(const std::string &message);
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

  // Log a message if it meets the minimum log level
  void log(const std::string &message, Level level);

  // Function to convert log level to a string label
  std::string getLabel(Level level);

  // Function to get current time as a string
  std::string getCurrentTime();

  // Logger state
  DECLARE_PROTECTED_MEMBER_WITH_SETTER(bool, ShouldPrintToConsole)

  /// @brief The mutex to lock the logger
  DECLARE_PROTECTED_MEMBER_NOGET(std::mutex, Mutex)

  /// @brief The file stream to write logs to a file
  DECLARE_PROTECTED_MEMBER_NOGET(std::ofstream, File)

  /// @brief The minimum log level that will be displayed
  DECLARE_PROTECTED_MEMBER(Level, LogLevel)
};

} // namespace STIMWALKER_NAMESPACE::utils

#endif // __STIMWALKER_UTILS_LOGGER_H__