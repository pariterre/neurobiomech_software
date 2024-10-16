#ifndef __STIMWALKER_UTILS_TEST_UTILS_H__
#define __STIMWALKER_UTILS_TEST_UTILS_H__

#include <vector>

#include "Utils/Logger.h"

static void
ASSERT_ALMOST_NOW(const std::chrono::system_clock::time_point &time,
                  const std::chrono::system_clock::time_point &now) {
  auto timeCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       time.time_since_epoch())
                       .count();
  auto nowCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now.time_since_epoch())
                      .count();
  auto nowDelayCount = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::milliseconds(50))
                           .count();
  ASSERT_GE(timeCount, nowCount);
  ASSERT_LE(timeCount, nowCount + nowDelayCount);
}

class TestLogger {
public:
  TestLogger() : m_logger(STIMWALKER_NAMESPACE::utils::Logger::getInstance()) {
    m_logger.setLogLevel(STIMWALKER_NAMESPACE::utils::Logger::INFO);
    m_logger.setShouldPrintToConsole(false);

    std::vector<std::string> &messagesToDevice(m_messagesToDevice);
    m_loggerId = m_logger.onNewLog.listen(
        [&messagesToDevice](const std::string &message) {
          messagesToDevice.push_back(message);
        });
  }
  ~TestLogger() { m_logger.onNewLog.clear(m_loggerId); }

  bool contains(const std::string &message) {
    for (const auto &msg : m_messagesToDevice) {
      if (msg.find(message) != std::string::npos) {
        return true;
      }
    }
    return false;
  }

  int count(const std::string &message) {
    int count = 0;
    for (const auto &msg : m_messagesToDevice) {
      if (msg.find(message) != std::string::npos) {
        count++;
      }
    }
    return count;
  }

  void clear() { m_messagesToDevice.clear(); }

protected:
  STIMWALKER_NAMESPACE::utils::Logger &m_logger;
  std::vector<std::string> m_messagesToDevice;
  size_t m_loggerId;
};

#endif // __STIMWALKER_UTILS_TEST_UTILS_H__