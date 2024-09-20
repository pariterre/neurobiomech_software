#include "Utils/Logger.h"

#include <filesystem>
#include <iomanip>
#include <iostream>

// Define the log levels
enum Level { INFO, WARNING, ERROR };

// Get the singleton instance of the Logger
Logger &Logger::getInstance() {
  static Logger instance;
  return instance;
}

void Logger::info(const std::string &message) { log(message, INFO); }

void Logger::warning(const std::string &message) { log(message, WARNING); }

void Logger::error(const std::string &message) { log(message, ERROR); }

// Set the minimum log level. Messages below this level will not be logged.
void Logger::setLogLevel(Level level) {
  std::lock_guard<std::mutex> lock(mutex_);
  minLogLevel_ = level;
}

// Set the log file to write logs to a file
void Logger::setLogFile(const std::string &filename) {
  std::lock_guard<std::mutex> lock(mutex_);

  std::filesystem::path filePath(filename);
  std::filesystem::path dirPath =
      filePath.parent_path(); // Get the directory path

  // Check if the directory exists, and create it if it does not
  if (!dirPath.empty() && !std::filesystem::exists(dirPath)) {
    // Create the directory (and parent directories if needed)
    if (!std::filesystem::create_directories(dirPath)) {
      std::cerr << "Failed to create directory: " << dirPath << std::endl;
      return; // Exit if directory creation failed
    }
  }

  if (file_.is_open()) {
    file_.close();
  }
  file_.open(filename, std::ios::out | std::ios::app);
}

// Private constructor and destructor to prevent direct instantiation
Logger::Logger() : minLogLevel_(INFO) {}

Logger::~Logger() {
  if (file_.is_open()) {
    file_.close();
  }
}

// Log a message if it meets the minimum log level
void Logger::log(const std::string &message, Level level) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Check if the current message meets the log level threshold
  if (level < minLogLevel_) {
    return; // Skip logging if below the minimum log level
  }

  // Get current time as a string
  std::string timeStamp = getCurrentTime();

  // Log to console
  std::ostream &os = (level == ERROR) ? std::cerr : std::cout;
  os << timeStamp << getLabel(level) << message << std::endl;

  // Log to file if logging to a file is enabled
  if (file_.is_open()) {
    file_ << timeStamp << getLabel(level) << message << std::endl;
  }
}

// Function to convert log level to a string label
std::string Logger::getLabel(Level level) {
  switch (level) {
  case INFO:
    return "[INFO]: ";
  case WARNING:
    return "[WARNING]: ";
  case ERROR:
    return "[ERROR]: ";
  default:
    return "[UNKNOWN]: ";
  }
}

// Function to get current time as a string
std::string Logger::getCurrentTime() {

  // Get current time as time_point
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);

  // Extract the milliseconds part
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) %
                1000;

  // Format the time as [YYYY-MM-DD HH:MM:SS.MMM]
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_time_t), "[%Y-%m-%d %H:%M:%S");
  ss << '.' << std::setfill('0') << std::setw(3)
     << now_ms.count(); // Add milliseconds
  ss << "] ";           // Space after the timestamp
  return ss.str();
}