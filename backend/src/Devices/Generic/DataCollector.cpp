#include "Devices/Generic/DataCollector.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::data;
using namespace STIMWALKER_NAMESPACE::devices;

DataCollector::DataCollector(
    size_t channelCount,
    const std::function<std::unique_ptr<data::TimeSeries>()>
        &timeSeriesGenerator)
    : m_DataChannelCount(channelCount), m_IsStreamingData(false),
      m_IsRecording(false), m_LiveTimeSeries(timeSeriesGenerator()),
      m_TrialTimeSeries(timeSeriesGenerator()) {
  m_LiveTimeSeries->setRollingVectorMaxSize(10);
}

bool DataCollector::startDataStreaming() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsStreamingData) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already streaming data");
    return true;
  }

  m_IsStreamingData = handleStartDataStreaming();
  m_HasFailedToStartDataStreaming = !m_IsStreamingData;
  m_LiveTimeSeries->reset();

  if (m_IsStreamingData) {
    logger.info("The data collector " + dataCollectorName() +
                " is now streaming data");
    return true;
  } else {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to start streaming data");
    return false;
  }
}

bool DataCollector::stopDataStreaming() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsStreamingData) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not streaming data");
    return true;
  }

  stopRecording();
  m_IsStreamingData = !handleStopDataStreaming();

  if (m_IsStreamingData) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to stop streaming data");
    return false;
  } else {
    logger.info("The data collector " + dataCollectorName() +
                " has stopped streaming data");
    return true;
  }
}

bool DataCollector::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsStreamingData) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " is not streaming data, so it cannot start recording");
    return false;
  }
  if (m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already recording");
    return true;
  }

  m_TrialTimeSeries->reset();
  m_IsRecording = true;

  logger.info("The data collector " + dataCollectorName() +
              " is now recording");
  return true;
}

bool DataCollector::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not recording");
    return true;
  }

  m_IsRecording = false;

  logger.info("The data collector " + dataCollectorName() +
              " has stopped recording");
  return true;
}

nlohmann::json DataCollector::getSerializedLiveData() const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_DataMutex));
  return m_LiveTimeSeries->serialize();
}

const TimeSeries &DataCollector::getLiveData() const {
  if (m_IsStreamingData) {
    std::string message =
        "The data collector " + dataCollectorName() + " is currently streaming";
    utils::Logger::getInstance().warning(message);
    throw DeviceDataNotAvailableException(message);
  }
  return *m_LiveTimeSeries;
}

const TimeSeries &DataCollector::getTrialData() const {
  if (m_IsRecording) {
    std::string message =
        "The data collector " + dataCollectorName() + " is currently recording";
    utils::Logger::getInstance().warning(message);
    throw DeviceDataNotAvailableException(message);
  }
  return *m_TrialTimeSeries;
}

void DataCollector::addDataPoints(
    const std::vector<std::vector<double>> &data) {
  if (!m_IsStreamingData || data.size() == 0) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_DataMutex);
  for (auto d : data) {
    m_LiveTimeSeries->add(d);
    if (m_IsRecording) {
      m_TrialTimeSeries->add(d);
    }
  }
  onNewData.notifyListeners(m_LiveTimeSeries->back());
}