#include "stimwalker.h"
#include <gtest/gtest.h>

#include "utils.h"

using namespace STIMWALKER_NAMESPACE;

TEST(Stimwalker, Version) { ASSERT_STREQ(STIMWALKER_VERSION, "0.1.0"); }

TEST(Logger, Messages) {
  // We use the TestLogger because if any of the tests fail, the memory is not
  // properly freed and segfaults happen. That said, we directly access the
  // logger the actual method. Also it automatically prepares a receiver for the
  // log messages which can be tested (this automatically tests [onNewLog])
  auto logger = TestLogger();

  // Send messages at INFO level (all should be received)
  logger.clear();
  auto &loggerInstance = utils::Logger::getInstance();
  loggerInstance.setLogLevel(utils::Logger::INFO);
  ASSERT_EQ(loggerInstance.getLogLevel(), utils::Logger::INFO);
  loggerInstance.info("This is an info message");
  loggerInstance.warning("This is a warning message");
  loggerInstance.fatal("This is an error message");

  ASSERT_TRUE(logger.contains("[INFO]: This is an info message"));
  ASSERT_TRUE(logger.contains("[WARNING]: This is a warning message"));
  ASSERT_TRUE(logger.contains("[FATAL]: This is an error message"));

  // Send messages at WARNING level (only warnings and errors should be
  // received)
  logger.clear();
  loggerInstance.setLogLevel(utils::Logger::WARNING);
  ASSERT_EQ(loggerInstance.getLogLevel(), utils::Logger::WARNING);
  loggerInstance.info("This is an info message");
  loggerInstance.warning("This is a warning message");
  loggerInstance.fatal("This is an error message");

  ASSERT_FALSE(logger.contains("[INFO]: This is an info message"));
  ASSERT_TRUE(logger.contains("[WARNING]: This is a warning message"));
  ASSERT_TRUE(logger.contains("[FATAL]: This is an error message"));

  // Send messages at FATAL level (only errors should be received)
  logger.clear();
  loggerInstance.setLogLevel(utils::Logger::FATAL);
  ASSERT_EQ(loggerInstance.getLogLevel(), utils::Logger::FATAL);
  loggerInstance.info("This is an info message");
  loggerInstance.warning("This is a warning message");
  loggerInstance.fatal("This is an error message");

  ASSERT_FALSE(logger.contains("[INFO]: This is an info message"));
  ASSERT_FALSE(logger.contains("[WARNING]: This is a warning message"));
  ASSERT_TRUE(logger.contains("[FATAL]: This is an error message"));
}

TEST(Logger, LogFile) {
  // We use the TestLogger because if any of the tests fail, the memory is not
  // properly freed and segfaults happen. That said, we directly access the
  // logger the actual method. Also it automatically prepares a receiver for the
  // log messages which can be tested (this automatically tests [onNewLog])
  auto logger = TestLogger();

  // Set the log file
  auto &loggerInstance = utils::Logger::getInstance();
  loggerInstance.setLogFile("test.log");

  // Check if the log file was created
  {
    std::ifstream file("test.log");
    ASSERT_TRUE(file.good());
    file.close();
  }

  // Send messages at INFO level (all should be received)
  loggerInstance.setLogLevel(utils::Logger::INFO);
  loggerInstance.info("This is an info message");
  loggerInstance.warning("This is a warning message");
  loggerInstance.fatal("This is an error message");
  // Send messages at WARNING level (only warnings and errors should be
  // received)
  loggerInstance.setLogLevel(utils::Logger::WARNING);
  loggerInstance.info("This is a second info message");
  loggerInstance.warning("This is a second warning message");
  loggerInstance.fatal("This is a second error message");
  // Send messages at FATAL level (only errors should be received)
  loggerInstance.setLogLevel(utils::Logger::FATAL);
  loggerInstance.info("This is a third info message");
  loggerInstance.warning("This is a third warning message");
  loggerInstance.fatal("This is a third error message");

  // Check if the log file contains (or skipped) the messages
  {
    std::string line;
    std::ifstream file("test.log");

    // INFO level
    std::getline(file, line);
    ASSERT_TRUE(line.find("[INFO]: This is an info message") !=
                std::string::npos);
    std::getline(file, line);
    ASSERT_TRUE(line.find("[WARNING]: This is a warning message") !=
                std::string::npos);
    std::getline(file, line);
    ASSERT_TRUE(line.find("[FATAL]: This is an error message") !=
                std::string::npos);

    // WARNING level
    std::getline(file, line);
    ASSERT_TRUE(line.find("[WARNING]: This is a second warning message") !=
                std::string::npos);
    std::getline(file, line);
    ASSERT_TRUE(line.find("[FATAL]: This is a second error message") !=
                std::string::npos);

    // FATAL level
    std::getline(file, line);
    ASSERT_TRUE(line.find("[FATAL]: This is a third error message") !=
                std::string::npos);

    file.close();
  }
}