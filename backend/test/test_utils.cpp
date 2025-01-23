#include "neurobio.h"
#include <gtest/gtest.h>

#include "utils.h"

#include "Utils/Logger.h"
#include "Utils/NeurobioEvent.h"
#include "Utils/RollingVector.h"

using namespace NEUROBIO_NAMESPACE;

TEST(Neurobio, Version) { ASSERT_STREQ(NEUROBIO_VERSION, "0.1.0"); }

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

TEST(NeurobioEvent, Calling) {
  // Setup a listener that changes a value to test if it is properly called
  utils::NeurobioEvent<int> event;
  int result = 0;
  auto id = event.listen([&result](int value) { result = value; });

  // Test that the listener is called
  event.notifyListeners(42);
  ASSERT_EQ(result, 42);

  // Remove the listener and test that it is not called anymore
  event.clear(id);
  event.notifyListeners(24);
  ASSERT_EQ(result, 42);
}

TEST(RollingVector, Adding) {
  auto vector = utils::RollingVector<int>(5);
  ASSERT_EQ(vector.getMaxSize(), 5);
  ASSERT_EQ(vector.size(), 0);

  vector.push_back(1);
  ASSERT_FALSE(vector.getIsFull());
  ASSERT_EQ(vector[0], 1);
  ASSERT_EQ(vector.at(0), 1);
  ASSERT_EQ(vector.size(), 1);

  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);
  vector.push_back(5);
  ASSERT_TRUE(vector.getIsFull());
  ASSERT_EQ(vector[0], 1);
  ASSERT_EQ(vector[4], 5);
  ASSERT_EQ(vector[5], 1); // Wraps
  ASSERT_EQ(vector.at(0), 1);
  ASSERT_EQ(vector.at(4), 5);
  EXPECT_THROW(vector.at(5), std::out_of_range);
  ASSERT_EQ(vector.size(), 5);
  size_t index = 0;
  for (auto &v : vector) {
    ASSERT_EQ(v, index % 5 + 1);
    index++;
  }
  ASSERT_EQ(index, 5);

  vector.push_back(6);
  ASSERT_EQ(vector[0], 2);
  ASSERT_EQ(vector[4], 6);
  ASSERT_EQ(vector[5], 2); // Wraps
  ASSERT_EQ(vector.at(0), 2);
  ASSERT_EQ(vector.at(4), 6);
  ASSERT_EQ(vector.at(5), 2);
  EXPECT_THROW(vector.at(6), std::out_of_range);
  ASSERT_EQ(vector.size(), 6);

  index = 0;
  for (auto &v : vector) {
    ASSERT_EQ(v, index % 5 + 2);
    index++;
  }
  ASSERT_EQ(index, 5);

  // Try it twice in a row
  index = 0;
  for (const auto &v : vector) {
    ASSERT_EQ(v, index % 5 + 2);
    index++;
  }
  ASSERT_EQ(index, 5);

  vector.clear();
  ASSERT_EQ(vector.getMaxSize(), 5);
  ASSERT_EQ(vector.size(), 0);
}

TEST(RollingVector, NoLimit) {
  auto vector = utils::RollingVector<int>();
  ASSERT_EQ(vector.getMaxSize(), size_t(-1));
  ASSERT_EQ(vector.size(), 0);

  vector.push_back(1);
  ASSERT_FALSE(vector.getIsFull());
  ASSERT_EQ(vector[0], 1);
  ASSERT_EQ(vector.at(0), 1);
  ASSERT_EQ(vector.size(), 1);

  vector.push_back(2);
  vector.push_back(3);
  vector.push_back(4);
  vector.push_back(5);
  ASSERT_FALSE(vector.getIsFull());
  ASSERT_EQ(vector[0], 1);
  ASSERT_EQ(vector[4], 5);
  ASSERT_EQ(vector.at(0), 1);
  ASSERT_EQ(vector.at(4), 5);
  EXPECT_THROW(vector.at(5), std::out_of_range);
  ASSERT_EQ(vector.size(), 5);

  vector.push_back(6);
  ASSERT_EQ(vector[0], 1);
  ASSERT_EQ(vector[4], 5);
  ASSERT_EQ(vector[5], 6);
  ASSERT_EQ(vector.at(0), 1);
  ASSERT_EQ(vector.at(4), 5);
  ASSERT_EQ(vector.at(5), 6);
  EXPECT_THROW(vector.at(6), std::out_of_range);
  ASSERT_EQ(vector.size(), 6);

  size_t index = 0;
  for (auto &v : vector) {
    ASSERT_EQ(v, index % 6 + 1);
    index++;
  }

  index = 0;
  for (const auto &v : vector) {
    ASSERT_EQ(v, index % 6 + 1);
    index++;
  }

  vector.clear();
  ASSERT_EQ(vector.getMaxSize(), size_t(-1));
  ASSERT_EQ(vector.size(), 0);
}

TEST(RollingVector, AllGetsWithMaxSize) {
  auto vector = utils::RollingVector<int>(5);
  ASSERT_EQ(vector.getMaxSize(), 5);
  ASSERT_EQ(vector.size(), 0);

  // When there are no elements
  ASSERT_NO_THROW(vector[0]);
  ASSERT_THROW(vector.at(0), std::out_of_range);
  ASSERT_THROW(vector.front(), std::out_of_range);
  ASSERT_THROW(vector.back(), std::out_of_range);
  size_t forLoopCount = 0;
  for (auto &v : vector) {
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 0);

  // When there is only one element
  vector.push_back(1);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 1);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 1);

  // When there are two elements
  vector.push_back(2);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector[1] == 2);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.at(1) == 2);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 2);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }

  // When there are exactly max size elements
  vector.push_back(3);
  vector.push_back(4);
  vector.push_back(5);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector[1] == 2);
  ASSERT_TRUE(vector[2] == 3);
  ASSERT_TRUE(vector[3] == 4);
  ASSERT_TRUE(vector[4] == 5);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.at(1) == 2);
  ASSERT_TRUE(vector.at(2) == 3);
  ASSERT_TRUE(vector.at(3) == 4);
  ASSERT_TRUE(vector.at(4) == 5);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 5);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 5);

  // When there is one element more than max size elements
  vector.push_back(6);
  ASSERT_TRUE(vector[0] == 2);
  ASSERT_TRUE(vector[1] == 3);
  ASSERT_TRUE(vector[2] == 4);
  ASSERT_TRUE(vector[3] == 5);
  ASSERT_TRUE(vector[4] == 6);
  ASSERT_TRUE(vector.at(0) == 2);
  ASSERT_TRUE(vector.at(1) == 3);
  ASSERT_TRUE(vector.at(2) == 4);
  ASSERT_TRUE(vector.at(3) == 5);
  ASSERT_TRUE(vector.at(4) == 6);
  ASSERT_TRUE(vector.front() == 2);
  ASSERT_TRUE(vector.back() == 6);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 2);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 5);

  // Where there is two elements more than max size elements
  vector.push_back(7);
  ASSERT_TRUE(vector[0] == 3);
  ASSERT_TRUE(vector[1] == 4);
  ASSERT_TRUE(vector[2] == 5);
  ASSERT_TRUE(vector[3] == 6);
  ASSERT_TRUE(vector[4] == 7);
  ASSERT_TRUE(vector.at(0) == 3);
  ASSERT_TRUE(vector.at(1) == 4);
  ASSERT_TRUE(vector.at(2) == 5);
  ASSERT_TRUE(vector.at(3) == 6);
  ASSERT_TRUE(vector.at(4) == 7);
  ASSERT_TRUE(vector.front() == 3);
  ASSERT_TRUE(vector.back() == 7);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 3);
    forLoopCount++;
  }

  // When there is exactly twice the max size elements
  vector.push_back(8);
  vector.push_back(9);
  vector.push_back(10);
  ASSERT_TRUE(vector[0] == 6);
  ASSERT_TRUE(vector[1] == 7);
  ASSERT_TRUE(vector[2] == 8);
  ASSERT_TRUE(vector[3] == 9);
  ASSERT_TRUE(vector[4] == 10);
  ASSERT_TRUE(vector.at(0) == 6);
  ASSERT_TRUE(vector.at(1) == 7);
  ASSERT_TRUE(vector.at(2) == 8);
  ASSERT_TRUE(vector.at(3) == 9);
  ASSERT_TRUE(vector.at(4) == 10);
  ASSERT_TRUE(vector.front() == 6);
  ASSERT_TRUE(vector.back() == 10);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 6);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 5);

  // When there is one element more than twice the max size elements
  vector.push_back(11);
  ASSERT_TRUE(vector[0] == 7);
  ASSERT_TRUE(vector[1] == 8);
  ASSERT_TRUE(vector[2] == 9);
  ASSERT_TRUE(vector[3] == 10);
  ASSERT_TRUE(vector[4] == 11);
  ASSERT_TRUE(vector.at(0) == 7);
  ASSERT_TRUE(vector.at(1) == 8);
  ASSERT_TRUE(vector.at(2) == 9);
  ASSERT_TRUE(vector.at(3) == 10);
  ASSERT_TRUE(vector.at(4) == 11);
  ASSERT_TRUE(vector.front() == 7);
  ASSERT_TRUE(vector.back() == 11);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 7);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 5);
}

TEST(RollingVector, AllGetsWithNoLimit) {
  auto vector = utils::RollingVector<int>();
  ASSERT_EQ(vector.getMaxSize(), size_t(-1));
  ASSERT_EQ(vector.size(), 0);

  // When there are no elements
  // ASSERT_THROW(vector[0], std::out_of_range); This is a segfault
  ASSERT_THROW(vector.at(0), std::out_of_range);
  ASSERT_THROW(vector.front(), std::out_of_range);
  ASSERT_THROW(vector.back(), std::out_of_range);
  int forLoopCount = 0;
  for (auto &v : vector) {
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 0);

  // When there is only one element
  vector.push_back(1);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 1);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 1);

  // When there are two elements
  vector.push_back(2);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector[1] == 2);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.at(1) == 2);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 2);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }

  // When there are multiple elements
  vector.push_back(3);
  vector.push_back(4);
  vector.push_back(5);
  ASSERT_TRUE(vector[0] == 1);
  ASSERT_TRUE(vector[1] == 2);
  ASSERT_TRUE(vector[2] == 3);
  ASSERT_TRUE(vector[3] == 4);
  ASSERT_TRUE(vector[4] == 5);
  ASSERT_TRUE(vector.at(0) == 1);
  ASSERT_TRUE(vector.at(1) == 2);
  ASSERT_TRUE(vector.at(2) == 3);
  ASSERT_TRUE(vector.at(3) == 4);
  ASSERT_TRUE(vector.at(4) == 5);
  ASSERT_TRUE(vector.front() == 1);
  ASSERT_TRUE(vector.back() == 5);
  forLoopCount = 0;
  for (auto &v : vector) {
    ASSERT_TRUE(v == forLoopCount + 1);
    forLoopCount++;
  }
  ASSERT_TRUE(forLoopCount == 5);
}