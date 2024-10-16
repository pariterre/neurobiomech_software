#include "Devices/Generic/DataCollector.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::data;
using namespace STIMWALKER_NAMESPACE::devices;

DataCollector::DataCollector(size_t channelCount,
                             std::unique_ptr<data::TimeSeries> timeSeries)
    : m_DataChannelCount(channelCount), m_IsStreamingData(false),
      m_IsPaused(false), m_TimeSeries(std::move(timeSeries)) {}

bool DataCollector::startDataStreaming() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsStreamingData) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already streaming data");
    return true;
  }

  m_IsStreamingData = handleStartDataStreaming();
  m_HasFailedToStartDataStreaming = !m_IsStreamingData;
  m_TimeSeries->clear();

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

void DataCollector::pauseRecording() { m_IsPaused = true; }

void DataCollector::resumeRecording() { m_IsPaused = false; }

const TimeSeries &DataCollector::getTimeSeries() const { return *m_TimeSeries; }

void DataCollector::addDataPoint(DataPoint &dataPoint) {
  if (!m_IsStreamingData || m_IsPaused) {
    return;
  }

  m_TimeSeries->add(dataPoint);
  onNewData.notifyListeners(dataPoint);
}

void DataCollector::addDataPoints(std::vector<DataPoint> &data) {
  if (!m_IsStreamingData || m_IsPaused) {
    return;
  }

  for (auto d : data) {
    m_TimeSeries->add(d);
  }
  onNewData.notifyListeners(data.back());
}