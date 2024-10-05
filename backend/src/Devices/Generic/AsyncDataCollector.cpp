#include "Devices/Generic/AsyncDataCollector.h"

#include "Utils/Logger.h"
#include <regex>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDataCollector::AsyncDataCollector(
    size_t channelCount, const std::chrono::milliseconds &dataCheckIntervals)
    : m_KeepDataWorkerAliveInterval(dataCheckIntervals),
      DataCollector(channelCount) {}

AsyncDataCollector::AsyncDataCollector(
    size_t channelCount, const std::chrono::microseconds &dataCheckIntervals)
    : m_KeepDataWorkerAliveInterval(dataCheckIntervals),
      DataCollector(channelCount) {}

void AsyncDataCollector::handleStartRecording() {
  bool isStartRecordingHandled = false;
  bool hasFailed = false;
  m_AsyncDataWorker =
      std::thread([this, &isStartRecordingHandled, &hasFailed]() {
        try {
          handleAsyncStartRecording();
        } catch (std::exception &) {
          hasFailed = true;
          return;
        }
        startKeepDataWorkerAlive();
        isStartRecordingHandled = true;
        m_AsyncDataContext.run();
      });

  // Wait until the connection is established
  while (!isStartRecordingHandled) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (hasFailed) {
      m_AsyncDataWorker.join();
      throw DeviceFailedToStartRecordingException(
          "Error while starting to record data");
    }
  }
}

void AsyncDataCollector::handleStopRecording() {
  // Just leave a bit of time if there are any pending commands to process
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // Give the hand to inherited classes to clean up some stuff
  handleAsyncStopRecording();

  // Stop the worker thread
  m_AsyncDataContext.stop();
  m_AsyncDataWorker.join();
}

void AsyncDataCollector::startKeepDataWorkerAlive() {
  m_KeepDataWorkerAliveTimer = std::make_unique<asio::steady_timer>(
      m_AsyncDataContext, m_KeepDataWorkerAliveInterval);
  keepDataWorkerAlive(m_KeepDataWorkerAliveInterval);
}
void AsyncDataCollector::keepDataWorkerAlive(
    std::chrono::milliseconds timeout) {
  keepDataWorkerAlive(std::chrono::microseconds(timeout));
}

void AsyncDataCollector::keepDataWorkerAlive(
    std::chrono::microseconds timeout) {
  // Set a timer that will call [pingDataWorker] every [timeout] milliseconds
  m_KeepDataWorkerAliveTimer->expires_after(timeout);

  m_KeepDataWorkerAliveTimer->async_wait([this](const asio::error_code &ec) {
    // If ec is not false, it means the timer was stopped to change the
    // interval, or the device was disconnected. In both cases, do nothing and
    // return
    if (ec)
      return;

    // Otherwise, send a PING command to the device
    std::lock_guard<std::mutex> lock(m_AsyncDataMutex);
    dataCheck();

    // Once its done, repeat the process
    keepDataWorkerAlive(m_KeepDataWorkerAliveInterval);
  });
}

void AsyncDataCollector::dataCheck() {}