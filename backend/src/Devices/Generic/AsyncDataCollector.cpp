#include "Devices/Generic/AsyncDataCollector.h"

#include "Utils/Logger.h"
#include <regex>
#include <thread>

using namespace STIMWALKER_NAMESPACE::devices;

AsyncDataCollector::AsyncDataCollector(
    size_t channelCount, const std::chrono::microseconds &dataCheckIntervals,
    std::unique_ptr<data::TimeSeries> timeSeries)
    : m_KeepDataWorkerAliveInterval(dataCheckIntervals),
      DataCollector(channelCount, std::move(timeSeries)) {}

void AsyncDataCollector::startRecordingAsync() {
  if (m_IsRecording) {
    utils::Logger::getInstance().warning(
        "The data collector " + dataCollectorName() + " is already recording");
    return;
  }

  m_HasFailedToStartRecording = false;
  m_AsyncDataWorker = std::thread([this]() {
    auto &logger = utils::Logger::getInstance();
    m_IsRecording = handleStartRecording();
    m_HasFailedToStartRecording = !m_IsRecording;

    if (m_HasFailedToStartRecording) {
      logger.fatal("The data collector " + dataCollectorName() +
                   " failed to start recording");
      return;
    }

    m_TimeSeries->clear();
    startKeepDataWorkerAlive();
    logger.info("The data collector " + dataCollectorName() +
                " is now recording");
    m_AsyncDataContext.run();
  });
}

bool AsyncDataCollector::startRecording() {
  startRecordingAsync();
  while (!m_IsRecording && !m_HasFailedToStartRecording) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (m_HasFailedToStartRecording) {
    stopDataCollectorWorkers();
    return false;
  }
  return true;
}

bool AsyncDataCollector::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not recording");
    return true;
  }

  // Cancel the timer first to ensure that it does not keep the io_context alive
  {
    std::lock_guard<std::mutex> lock(m_AsyncDataMutex);
    m_IsRecording = false;
  }
  std::this_thread::sleep_for(m_KeepDataWorkerAliveInterval);

  // Give the hand to inherited classes to clean up some stuff
  m_IsRecording = !handleStopRecording();
  if (m_IsRecording) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to stop recording");
    return false;
  }

  stopDataCollectorWorkers();
  logger.info("The data collector " + dataCollectorName() +
              " has stopped recording");
  return true;
}

void AsyncDataCollector::stopDataCollectorWorkers() {
  if (m_IsRecording) {
    stopRecording();
  }

  // Stop the worker thread
  if (!m_AsyncDataContext.stopped()) {
    m_AsyncDataContext.stop();
  }

  if (m_AsyncDataWorker.joinable()) {
    m_AsyncDataWorker.join();
  }
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
