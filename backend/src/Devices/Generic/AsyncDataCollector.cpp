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

        m_IsRecording = true;
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
  // Cancel the timer first to ensure that it does not keep the io_context alive
  std::lock_guard<std::mutex> lock(m_AsyncDataMutex);
  m_IsRecording = false;
  std::this_thread::sleep_for(m_KeepDataWorkerAliveInterval);

  // Stop the worker thread
  m_AsyncDataContext.stop();
  m_AsyncDataWorker.join();

  // Give the hand to inherited classes to clean up some stuff
  handleAsyncStopRecording();
}

void AsyncDataCollector::startKeepDataWorkerAlive() {
  m_KeepDataWorkerAliveTimer = std::make_unique<asio::steady_timer>(
      m_AsyncDataContext, m_KeepDataWorkerAliveInterval);
  keepDataWorkerAlive(m_KeepDataWorkerAliveInterval);
}
void AsyncDataCollector::keepDataWorkerAlive(
    std::chrono::milliseconds timeout) {
  keepDataWorkerAlive(
      std::chrono::duration_cast<std::chrono::microseconds>(timeout));
}

void AsyncDataCollector::keepDataWorkerAlive(
    std::chrono::microseconds timeout) {

  // Set a timer that will call [pingDataWorker] every [timeout] milliseconds
  m_KeepDataWorkerAliveTimer->expires_after(timeout);

  m_KeepDataWorkerAliveTimer->async_wait([this](const auto &errorCode) {
    // Get the current time
    auto now = std::chrono::steady_clock::now();

    // If errorCode is not false, it means the timer was stopped by the user, or
    // the device was disconnected. In both cases, do nothing and return
    if (!m_IsRecording || errorCode) {
      m_KeepDataWorkerAliveTimer->cancel();
      return;
    }

    // Otherwise, send a [dataCheck] command to allow the user to check for
    // new data
    std::lock_guard<std::mutex> lock(m_AsyncDataMutex);
    dataCheck();

    // Once it's done, repeat the process, but take into account the time it
    // took to execute the [dataCheck] method
    auto timeToExecute = std::chrono::steady_clock::now() - now;
    auto next = m_KeepDataWorkerAliveInterval - timeToExecute;
    if (next < std::chrono::microseconds(1)) {
      next = std::chrono::microseconds(1);

      // Send a warning to the user if the delay is more than twice the
      // interval
      if (!m_IgnoreTooSlowWarning) {
        utils::Logger::getInstance().warning(
            "The [dataCheck] for " + dataCollectorName() +
            " took longer than the sampling rate (" +
            std::to_string(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    timeToExecute)
                    .count()) +
            "/" + std::to_string(m_KeepDataWorkerAliveInterval.count()) +
            " microseconds). Consider increasing the interval, or optimizing "
            "the [dataCheck] method.");
      }
    }

    keepDataWorkerAlive(
        std::chrono::duration_cast<std::chrono::microseconds>(next));
  });
}

void AsyncDataCollector::dataCheck() {}