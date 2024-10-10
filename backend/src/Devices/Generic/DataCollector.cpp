#include "Devices/Generic/DataCollector.h"

#include "Devices/Exceptions.h"
#include "Utils/Logger.h"

using namespace STIMWALKER_NAMESPACE::data;
using namespace STIMWALKER_NAMESPACE::devices;

DataCollector::DataCollector(size_t channelCount,
                             std::unique_ptr<data::TimeSeries> timeSeries)
    : m_DataChannelCount(channelCount), m_IsRecording(false),
      m_TimeSeries(std::move(timeSeries)) {}

void DataCollector::startRecording() {
  auto &logger = utils::Logger::getInstance();

  if (m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is already recording");
    return;
  }

  if (!handleStartRecording()) {
    logger.fatal("The data collector " + dataCollectorName() +
                 " failed to start recording");
    return;
  }

  m_TimeSeries->clear();
  m_IsRecording = true;
  logger.info("The data collector " + dataCollectorName() +
              " is now recording");
}

void DataCollector::stopRecording() {
  auto &logger = utils::Logger::getInstance();

  if (!m_IsRecording) {
    logger.warning("The data collector " + dataCollectorName() +
                   " is not recording");
    return;
  }

  handleStopRecording();

  m_IsRecording = false;
  logger.info("The data collector " + dataCollectorName() +
              " has stopped recording");
}

const TimeSeries &DataCollector::getTrialData() const { return *m_TimeSeries; }

void DataCollector::addDataPoint(const DataPoint &dataPoint) {
  if (!m_IsRecording) {
    return;
  }

  m_TimeSeries->add(dataPoint);
  onNewData.notifyListeners(dataPoint);
}

void DataCollector::addDataPoints(const std::vector<DataPoint> &data) {
  if (!m_IsRecording) {
    return;
  }

  for (auto d : data) {
    m_TimeSeries->add(d);
  }
  onNewData.notifyListeners(data.back());
}