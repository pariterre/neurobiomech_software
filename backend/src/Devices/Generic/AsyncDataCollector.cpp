#include "Devices/Generic/AsyncDataCollector.h"

#include "Utils/Logger.h"
#include <regex>
#include <thread>

using namespace NEUROBIO_NAMESPACE::devices;

AsyncDataCollector::AsyncDataCollector(
    size_t channelCount, const std::chrono::microseconds &dataCheckIntervals,
    const std::function<std::unique_ptr<data::TimeSeries>()>
        &timeSeriesGenerator)
    : m_KeepDataWorkerAliveInterval(dataCheckIntervals),
      DataCollector(channelCount, timeSeriesGenerator) {}

AsyncDataCollector::~AsyncDataCollector() { stopDataCollectorWorkers(); }

void AsyncDataCollector::startDataStreamingAsync() {
  if (m_IsStreamingData) {
    utils::Logger::getInstance().warning("The data collector " +
                                         dataCollectorName() +
                                         " is already streaming data");
    return;
  }

  m_AsyncDataContext.restart();
  m_HasFailedToStartDataStreaming = false;
  m_AsyncDataWorker = std::thread([this]() {
    auto &logger = utils::Logger::getInstance();
    m_HasFailedToStartDataStreaming = !handleStartDataStreaming();

    if (m_HasFailedToStartDataStreaming) {
      m_IsStreamingData = false;
      logger.fatal("The data collector " + dataCollectorName() +
                   " failed to start streaming datas");
      return;
    }

    m_LiveTimeSeries->reset();
    startKeepDataWorkerAlive();
    m_IsStreamingData = true;
    logger.info("The data collector " + dataCollectorName() +
                " is now streaming data");
    m_AsyncDataContext.run();
  });
}

bool AsyncDataCollector::startDataStreaming() {
  startDataStreamingAsync();
  while (!m_IsStreamingData && !m_HasFailedToStartDataStreaming) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (m_HasFailedToStartDataStreaming) {
    stopDataCollectorWorkers();
    return false;
  }
  return true;
}

bool AsyncDataCollector::stopDataStreaming() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsStreamingData && !m_HasFailedToStartDataStreaming) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not streaming data");
    return true;
  }

  // Cancel the timer first to ensure that it does not keep the io_context alive
  {
    std::unique_lock lock(m_AsyncDataMutex);
    m_IsStreamingData = false;
    m_HasFailedToStartDataStreaming = false;
  }
  std::this_thread::sleep_for(m_KeepDataWorkerAliveInterval);

  // Give the hand to inherited classes to clean up some stuff
  stopRecording();
  bool success = handleStopDataStreaming();
  if (!success) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to stop streaming data");
  }

  stopDataCollectorWorkers();
  logger.info("The data collector " + dataCollectorName() +
              " has stopped streaming data");
  return true;
}

bool AsyncDataCollector::startRecording() {
  std::unique_lock lock(m_AsyncDataMutex);
  return DataCollector::startRecording();
}

bool AsyncDataCollector::stopRecording() {
  std::unique_lock lock(m_AsyncDataMutex);
  return DataCollector::stopRecording();
}

void AsyncDataCollector::stopDataCollectorWorkers() {
  if (m_IsRecording) {
    stopRecording();
  }

  if (m_IsStreamingData) {
    stopDataStreaming();
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
    auto now = std::chrono::high_resolution_clock::now();

    // If errorCode is not false, it means the timer was stopped by the user, or
    // the device was disconnected. In both cases, do nothing and return
    if (!m_IsStreamingData || errorCode) {
      m_KeepDataWorkerAliveTimer->cancel();
      return;
    }

    // Otherwise, send a [dataCheck] command to allow the user to check for
    // new data
    dataCheck();

    // Once it's done, repeat the process, but take into account the time it
    // took to execute the [dataCheck] method
    auto timeToExecute = std::chrono::high_resolution_clock::now() - now;
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
